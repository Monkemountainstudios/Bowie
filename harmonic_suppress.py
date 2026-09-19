from pathlib import Path
import argparse
import wave

import numpy as np


def read_pcm16(path: Path):
    with wave.open(str(path), "rb") as wav:
        params = wav.getparams()
        if params.sampwidth != 2 or params.comptype != "NONE":
            raise ValueError("Expected an uncompressed 16-bit PCM WAV")
        raw = wav.readframes(params.nframes)
    audio = np.frombuffer(raw, dtype="<i2").astype(np.float64)
    return params, (audio / 32768.0).reshape(-1, params.nchannels)


def write_pcm16(path: Path, params, audio):
    path.parent.mkdir(parents=True, exist_ok=True)
    pcm = np.round(np.clip(audio, -1.0, 1.0) * 32767.0).astype("<i2")
    with wave.open(str(path), "wb") as wav:
        wav.setnchannels(params.nchannels)
        wav.setsampwidth(2)
        wav.setframerate(params.framerate)
        wav.writeframes(pcm.tobytes())


def suppression_curve(sample_rate, fft_size):
    frequencies = np.fft.rfftfreq(fft_size, 1.0 / sample_rate)
    curve = np.ones_like(frequencies)
    for fundamental in (323.0, 449.5):
        center = fundamental
        while center <= 6000.0:
            half_width = max(28.0, center * 0.025)
            distance = np.abs(frequencies - center) / half_width
            # -36 dB at each moving harmonic, with a raised-cosine shoulder.
            attenuation = np.ones_like(curve)
            core = distance <= 1.0
            shoulder = (distance > 1.0) & (distance < 2.0)
            attenuation[core] = 10.0 ** (-36.0 / 20.0)
            x = distance[shoulder] - 1.0
            attenuation[shoulder] = 0.5 - 0.5 * np.cos(np.pi * x)
            curve *= attenuation
            center += fundamental
    curve *= np.clip((frequencies - 70.0) / 120.0, 0.0, 1.0)
    return curve


def process(audio, sample_rate, fft_size=8192, hop=2048):
    window = np.hanning(fft_size)
    curve = suppression_curve(sample_rate, fft_size)
    pad = fft_size
    padded = np.pad(audio, ((pad, pad), (0, 0)))
    output = np.zeros_like(padded)
    weight = np.zeros(len(padded))
    for start in range(0, len(padded) - fft_size + 1, hop):
        frame = padded[start:start + fft_size] * window[:, None]
        spectrum = np.fft.rfft(frame, axis=0)
        restored = np.fft.irfft(spectrum * curve[:, None], n=fft_size, axis=0)
        output[start:start + fft_size] += restored * window[:, None]
        weight[start:start + fft_size] += window**2
    output /= np.maximum(weight[:, None], 1.0e-12)
    return output[pad:pad + len(audio)]


parser = argparse.ArgumentParser()
parser.add_argument("source", type=Path)
parser.add_argument("destination", type=Path)
args = parser.parse_args()

params, source = read_pcm16(args.source)
result = process(source, params.framerate)
result *= (10.0 ** (-6.0 / 20.0)) / max(np.max(np.abs(result)), 1.0e-12)
write_pcm16(args.destination, params, result)
print(args.destination.name)
