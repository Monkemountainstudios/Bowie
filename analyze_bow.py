from pathlib import Path
import sys
import wave

import numpy as np


def read_mono(path: Path):
    with wave.open(str(path), "rb") as wav:
        rate = wav.getframerate()
        channels = wav.getnchannels()
        width = wav.getsampwidth()
        raw = wav.readframes(wav.getnframes())
    if width != 2:
        raise ValueError(f"Only 16-bit PCM is supported, got {width * 8}-bit")
    data = np.frombuffer(raw, dtype="<i2").astype(np.float64) / 32768.0
    data = data.reshape(-1, channels)
    mono = np.mean(data, axis=1)
    return rate, channels, mono


def db(value):
    return 20.0 * np.log10(max(float(value), 1.0e-12))


for arg in sys.argv[1:]:
    path = Path(arg)
    rate, channels, audio = read_mono(path)
    duration = len(audio) / rate
    peak = np.max(np.abs(audio))
    rms = np.sqrt(np.mean(audio**2))

    # Ignore the first and last 10% when identifying sustained tonal peaks.
    lo = int(len(audio) * 0.10)
    hi = int(len(audio) * 0.90)
    section = audio[lo:hi] if hi > lo else audio
    fft_size = min(16384, 2 ** int(np.floor(np.log2(max(len(section), 2)))))
    hop = max(1, fft_size // 2)
    window = np.hanning(fft_size)
    spectra = []
    for start in range(0, max(1, len(section) - fft_size + 1), hop):
        frame = section[start:start + fft_size]
        if len(frame) < fft_size:
            frame = np.pad(frame, (0, fft_size - len(frame)))
        spectra.append(np.abs(np.fft.rfft(frame * window)) ** 2)
    power = np.mean(spectra, axis=0)
    freqs = np.fft.rfftfreq(fft_size, 1.0 / rate)
    mask = (freqs >= 40.0) & (freqs <= 10000.0)
    f = freqs[mask]
    p = power[mask]
    logp = 10.0 * np.log10(np.maximum(p, 1.0e-30))
    peaks = np.flatnonzero((logp[1:-1] > logp[:-2]) & (logp[1:-1] >= logp[2:])) + 1
    ranked = peaks[np.argsort(p[peaks])[::-1]][:12]

    print(path.name)
    print(f"  {rate} Hz, {channels} ch, {duration:.3f} s, peak {db(peak):.1f} dBFS, RMS {db(rms):.1f} dBFS")
    peak_text = []
    for i in ranked:
        left = max(0, i - 8)
        right = min(len(p), i + 9)
        floor = np.median(p[left:right])
        prominence = 10.0 * np.log10(max(p[i] / max(floor, 1.0e-30), 1.0e-30))
        peak_text.append(f"{f[i]:.1f} Hz (+{prominence:.1f} dB)")
    print("  dominant peaks: " + ", ".join(peak_text))
