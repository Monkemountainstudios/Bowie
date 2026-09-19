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


def accumulated_power(audio, fft_size, hop):
    window = np.hanning(fft_size)
    power = np.zeros(fft_size // 2 + 1)
    for start in range(0, max(1, len(audio) - fft_size + 1), hop):
        frame = audio[start:start + fft_size]
        if len(frame) < fft_size:
            frame = np.pad(frame, ((0, fft_size - len(frame)), (0, 0)))
        spectrum = np.fft.rfft(frame * window[:, None], axis=0)
        power += np.sum(np.abs(spectrum) ** 2, axis=1)
    return power


def reference_curve(before, after, fft_size=8192, hop=2048):
    before_power = accumulated_power(before, fft_size, hop)
    after_power = accumulated_power(after, fft_size, hop)
    gain_db = 10.0 * np.log10((after_power + 1.0e-20) / (before_power + 1.0e-20))

    # Smooth enough to reproduce broad EQ while avoiding source-specific FFT spikes.
    width = 17
    kernel = np.ones(width) / width
    gain_db = np.convolve(gain_db, kernel, mode="same")
    gain_db = np.clip(gain_db, -48.0, 3.0)
    return 10.0 ** (gain_db / 20.0), gain_db


def stft_filter(audio, gain, fft_size=8192, hop=2048):
    window = np.hanning(fft_size)
    pad = fft_size
    padded = np.pad(audio, ((pad, pad), (0, 0)))
    output = np.zeros_like(padded)
    weight = np.zeros(len(padded))
    for start in range(0, len(padded) - fft_size + 1, hop):
        frame = padded[start:start + fft_size] * window[:, None]
        spectrum = np.fft.rfft(frame, axis=0)
        restored = np.fft.irfft(spectrum * gain[:, None], n=fft_size, axis=0)
        output[start:start + fft_size] += restored * window[:, None]
        weight[start:start + fft_size] += window**2
    output /= np.maximum(weight[:, None], 1.0e-12)
    return output[pad:pad + len(audio)]


parser = argparse.ArgumentParser()
parser.add_argument("reference_before", type=Path)
parser.add_argument("reference_after", type=Path)
parser.add_argument("source", type=Path)
parser.add_argument("destination", type=Path)
args = parser.parse_args()

before_params, before = read_pcm16(args.reference_before)
after_params, after = read_pcm16(args.reference_after)
source_params, source = read_pcm16(args.source)
if before_params.framerate != after_params.framerate or before_params.framerate != source_params.framerate:
    raise ValueError("All recordings must use the same sample rate")

gain, gain_db = reference_curve(before, after)
result = stft_filter(source, gain)

# Give the comparison file useful headroom without changing its spectral balance.
target_peak = 10.0 ** (-6.0 / 20.0)
peak = np.max(np.abs(result))
result *= target_peak / max(peak, 1.0e-12)
write_pcm16(args.destination, source_params, result)

frequencies = np.fft.rfftfreq(8192, 1.0 / source_params.framerate)
checks = (100, 200, 323, 450, 650, 900, 1300, 2000, 4000, 8000, 12000)
summary = ", ".join(
    f"{frequency} Hz: {gain_db[np.argmin(np.abs(frequencies-frequency))]:.1f} dB"
    for frequency in checks
)
print(args.destination.name)
print(summary)
