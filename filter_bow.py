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
    audio = (audio / 32768.0).reshape(-1, params.nchannels)
    return params, audio


def write_pcm16(path: Path, params, audio):
    path.parent.mkdir(parents=True, exist_ok=True)
    pcm = np.round(np.clip(audio, -1.0, 1.0) * 32767.0).astype("<i2")
    with wave.open(str(path), "wb") as wav:
        wav.setnchannels(params.nchannels)
        wav.setsampwidth(2)
        wav.setframerate(params.framerate)
        wav.writeframes(pcm.tobytes())


def biquad(audio, b0, b1, b2, a1, a2):
    output = np.empty_like(audio)
    for channel in range(audio.shape[1]):
        x1 = x2 = y1 = y2 = 0.0
        for index, x0 in enumerate(audio[:, channel]):
            y0 = b0 * x0 + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2
            output[index, channel] = y0
            x2, x1 = x1, x0
            y2, y1 = y1, y0
    return output


def notch(audio, sample_rate, frequency, q):
    omega = 2.0 * np.pi * frequency / sample_rate
    alpha = np.sin(omega) / (2.0 * q)
    a0 = 1.0 + alpha
    return biquad(
        audio,
        1.0 / a0,
        -2.0 * np.cos(omega) / a0,
        1.0 / a0,
        -2.0 * np.cos(omega) / a0,
        (1.0 - alpha) / a0,
    )


def highpass(audio, sample_rate, frequency, q=0.7071):
    omega = 2.0 * np.pi * frequency / sample_rate
    alpha = np.sin(omega) / (2.0 * q)
    cosine = np.cos(omega)
    a0 = 1.0 + alpha
    return biquad(
        audio,
        ((1.0 + cosine) / 2.0) / a0,
        -(1.0 + cosine) / a0,
        ((1.0 + cosine) / 2.0) / a0,
        (-2.0 * cosine) / a0,
        (1.0 - alpha) / a0,
    )


def filter_harmonics(audio, sample_rate):
    filtered = highpass(audio, sample_rate, 120.0)
    for fundamental in (323.0, 449.5):
        harmonic = fundamental
        while harmonic <= 6000.0:
            # A deliberately broad rejection follows the small pitch wander in
            # the bowed tone instead of leaving audible shoulders around it.
            filtered = notch(filtered, sample_rate, harmonic, q=7.0)
            harmonic += fundamental
    return filtered


parser = argparse.ArgumentParser()
parser.add_argument("source", type=Path)
parser.add_argument("destination", type=Path)
parser.add_argument("--highpass-only", action="store_true",
                    help="Apply only a gentle high-pass filter")
parser.add_argument("--cutoff", type=float, default=120.0,
                    help="High-pass cutoff in Hz (default: 120)")
parser.add_argument("--preserve-gain", action="store_true",
                    help="Do not peak-normalise after filtering")
args = parser.parse_args()

params, source = read_pcm16(args.source)
result = (highpass(source, params.framerate, args.cutoff)
          if args.highpass_only
          else filter_harmonics(source, params.framerate))
makeup_gain = 1.0
if not args.preserve_gain:
    filtered_peak = np.max(np.abs(result))
    target_peak = 10.0 ** (-6.0 / 20.0)
    makeup_gain = target_peak / max(filtered_peak, 1.0e-12)
    result *= makeup_gain
write_pcm16(args.destination, params, result)

source_rms = np.sqrt(np.mean(source**2))
result_rms = np.sqrt(np.mean(result**2))
print(
    f"{args.destination.name}: makeup {20*np.log10(makeup_gain):.1f} dB, "
    f"output RMS {20*np.log10(result_rms):.1f} dBFS"
)
