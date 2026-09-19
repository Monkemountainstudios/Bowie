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


def detone(audio, sample_rate, fft_size=4096, hop=512):
    window = np.hanning(fft_size)
    pad = fft_size
    padded = np.pad(audio, ((pad, pad), (0, 0)))
    starts = range(0, len(padded) - fft_size + 1, hop)
    frames = np.stack([padded[s:s + fft_size] * window[:, None] for s in starts])
    spectra = np.fft.rfft(frames, axis=1)

    # Use a shared stereo magnitude so filtering does not pull the image around.
    magnitude = np.sqrt(np.mean(np.abs(spectra) ** 2, axis=2))
    width = 17
    half = width // 2
    padded_magnitude = np.pad(magnitude, ((0, 0), (half, half)), mode="reflect")
    neighborhoods = np.lib.stride_tricks.sliding_window_view(
        padded_magnitude, width, axis=1
    )
    local_floor = np.median(neighborhoods, axis=-1) + 1.0e-12

    # Narrow spectral ridges are pitched tone; broadband energy is bow friction.
    ratio = magnitude / local_floor
    threshold = 1.45
    gain = np.minimum(1.0, (threshold / np.maximum(ratio, threshold)) ** 1.8)
    gain = np.maximum(gain, 0.025)

    # Remove rumble gradually without stripping the useful body of the scrape.
    frequencies = np.fft.rfftfreq(fft_size, 1.0 / sample_rate)
    low_cut = np.clip((frequencies - 80.0) / 100.0, 0.0, 1.0)
    gain *= low_cut[None, :]
    spectra *= gain[:, :, None]

    restored = np.fft.irfft(spectra, n=fft_size, axis=1)
    output = np.zeros_like(padded)
    weight = np.zeros(len(padded))
    for frame, start in zip(restored, starts):
        output[start:start + fft_size] += frame * window[:, None]
        weight[start:start + fft_size] += window**2
    output /= np.maximum(weight[:, None], 1.0e-12)
    return output[pad:pad + len(audio)]


parser = argparse.ArgumentParser()
parser.add_argument("source", type=Path)
parser.add_argument("destination", type=Path)
args = parser.parse_args()

params, source = read_pcm16(args.source)
result = detone(source, params.framerate)
peak = np.max(np.abs(result))
result *= (10.0 ** (-6.0 / 20.0)) / max(peak, 1.0e-12)
write_pcm16(args.destination, params, result)

source_rms = np.sqrt(np.mean(source**2))
result_rms = np.sqrt(np.mean(result**2))
print(f"{args.destination.name}: output RMS {20*np.log10(result_rms):.1f} dBFS")
