# Bowie

JUCE instrument that treats sampled body tones as if they were being bowed.

> **Beta:** Bowie 0.4.5 is ready for Windows and macOS testing. Builds are
> unsigned; Windows may show a SmartScreen warning and macOS may require
> control-clicking the app and choosing Open on first launch.

## Current voice model

- Fifteen selectable body models: Bassoon, Church, English Horn, Flute, Horn,
  Piano, Rhodes, Trumpet, Tuba, Vibra, Harp, Harpsichord, Oboe, Farfisa, and
  Marimba.
- Each model uses its real recorded octave anchors rather than stretching one
  sample across the full keyboard. Adjacent anchors are blended continuously
  with equal-power gains over the octave, avoiding a hard timbre switch.
- Flute now uses C3/C4/C5. English Horn intentionally uses only C3/C4, matching
  the useful real-world range represented by the source recordings; notes
  outside that span use the nearest anchor.
- Harp uses C3/C5 anchors, Harpsichord uses C2/C3/C4, and Oboe uses its recorded
  B-flat 2/3/4 anchors. Harp and Harpsichord retain their natural decays before
  entering the late sustain loop.
- Farfisa uses stable C2/C3/C4 sustain loops. Marimba uses C2/C3/C4, preserves
  each complete mallet strike, and crossfades into a compact mid-decay resonance
  loop rather than repeating the transient.
- Per-anchor RMS compensation is deliberately bounded, reducing distracting
  level changes without flattening each instrument's natural dynamics.
- Independent two-head loops for the body and high-pass-filtered six-second
  long-bow texture. Loop
  windows and each incoming head position vary from note to note and cycle to
  cycle to avoid a mechanically repeating seam.
- The gentle and scraping bow catches use an equal-power velocity transition
  centred near two thirds of the MIDI velocity range, with bounded loudness.
- The gentle catch is high-pass filtered at 250 Hz so it contributes movement
  without adding low-frequency body; its playback tail may be shortened later.
- The untouched short bow-noise catch is reserved for harder, more staccato
  playing at the upper end of the velocity transition.
- One shared velocity gain is applied after the tone/bow blend, so both layers
  respond as a single physical gesture instead of changing level independently.
- Bow level defaults to the tested 0.022 sweet spot, with a focused 0.20
  maximum for deliberately dirty textures. The continuous bed is attenuated
  independently so the velocity-selected attacks remain audible.
- Velocity controls the depth of a decaying, irregular attack pitch strain.
- A permanent 0.46 Hz / 1.65 cent drift adds a barely perceptible human motion.
- MIDI CC1 adds vibrato. Depth rises moderately; speed changes little through
  the first half of the wheel, then accelerates toward a restrained 10.2 Hz
  ceiling at maximum. CC1 is held in one instrument-wide state, so new and
  recycled voices always inherit the current wheel position and zero reaches
  every note consistently. Its oscillator is sample-synchronous across all
  active voices, preventing legato notes from beating against or cancelling
  one another.
- Pitch strain uses slower note-level randomized rates, decay, depth, and
  initial pull, so repeated gestures retain the same character without tracing
  the exact same motion.
- Piano, Rhodes, Harp, and Harpsichord play their recorded decays before entering
  later, compact tail loops with a longer crossfade, producing a quieter
  pad-like sustain instead of an early echo.
- The continuous bow bed follows Piano, Rhodes, Vibra, Harp, Harpsichord, and Marimba
  downward instead of remaining exposed after those naturally decaying bodies
  have settled. When layering, it fades only if every audible body decays.
- Vibra's C2 anchor retains its complete hammer at zero attack, then advances
  progressively past the longer hammer tail as ATTACK rises so its response
  agrees with the two upper anchors.
- The three Tuba recordings are mapped to their measured C-sharp fundamentals
  (C-sharp 2/3/4), correcting their previous one-semitone offset without
  altering the source audio.
- A post-voice 12 dB low-pass provides cutoff and softly bounded resonance.
  A compact stereo room follows it at a fixed 30 percent setting; additional
  ambience is intentionally left to the host.
- All utility text uses the embedded Bruno Ace typeface; the Bowie wordmark
  uses the embedded Fancytext display face.
- The instrument body fades from warm ivory at the top into royal blue at the
  bottom; panel labels use a fine ivory keyline and dark shadow for contrast.
- Twelve-note polyphony with three independently selectable body layers per
  note, smooth V1/V2/V3 gains, final master gain, individual per-layer ADSR,
  bow level, pitch strain, pitch wheel, and an on-screen keyboard. A compact
  three-position switch beside the ADSR bank selects the envelope being edited.
  V1 defaults to the original single-body sound while V2 and V3 begin muted.
- A zero-volume layer remains selectable but its VOICE label and body menu dim
  to 40 percent opacity, making inactive layers immediately legible without a
  redundant `No Tone` menu entry.
- The factory preset menu includes `Flute`, `Into the Forest`, `Saloon`, `Fanfare`,
  `Dirty Train`, `3 Flutes`, and `Tape E-Piano`.
  Each recalls all three bodies plus all three envelopes, bow, strain,
  filter, resonance, layer balance, and master settings.
- `Fanfare` combines Farfisa, Oboe, and Trumpet with a separate envelope for
  each layer, pairing the Farfisa's bright edge with softer winds and brass.
- The supplied multi-instrument artwork is used for the app/plugin icon and in
  the interface. Its ivory, gold, royal blue, red, and black palette now drives
  the instrument UI. Release artwork lives in `Assets/BowieArtwork.png`.
- All continuous controls use vertical bottom-to-top faders with symmetric
  21-mark ruler scales and minimal normalized `0`/`1` endpoints. The bottom,
  midpoint, and top marks are longest, with alternating minor lengths between
  them in the ARP-inspired layout. Numerical value boxes are intentionally
  omitted from the hardware-like panel.
- The cutoff fader uses a dedicated fine mouse-wheel response for smooth tonal
  adjustments across its skewed frequency range.
- A stepped gold divider groups the compact ADSR bank, central bow/filter
  shaping controls, and the tightly spaced VOL 1/2/3/Master bank without extra
  section titles. Each ruler now spans the fader's exact endpoint-to-endpoint
  travel.
- The standalone app discards a persisted audio setup when its output endpoint
  is blank. Recovery prefers native ASIO on Windows, then shared Windows Audio;
  macOS falls back to CoreAudio. Only one standalone Bowie instance may run at
  once, avoiding duplicate instances competing for the same driver.
- A fixed 6 ms safety ramp prevents waveform discontinuities even when the
  musical Attack control is set to zero.
- Each factory envelope is Attack 0.046 s, Decay 0.453 s, Sustain 0.455, and
  Release 0.238 s. A 12 ms continuous follower rounds every ADSR hand-off and
  live parameter change so stage boundaries glide rather than step.

The earlier `bow_layered_balanced_-12dB.wav` composite is retained in `Samples/`
as a listening reference but is not embedded in the instrument.

## Beta packages

Every push to `main` builds and tests Windows and macOS packages in GitHub
Actions. Tags beginning with `v` additionally publish both ZIP files as a
GitHub prerelease. The VST3 bundle and standalone app are included in each ZIP.

The macOS beta is universal (`arm64` and `x86_64`) but is not yet signed or
notarized.

## Build

Clone JUCE 9.0.1 beside the project as `JUCE`, or pass its location with
`-DBOWIE_JUCE_SOURCE_DIR=...`. On this development machine, `F:/JUCE` remains
an automatic fallback.

```powershell
cmake -S . -B build -G "Visual Studio 18 2026" -A x64
cmake --build build --config Release
```

Builds both VST3 and standalone targets. Embedded WAV assets live in `Samples/`.

Font licensing notices are stored beside the embedded fonts in `Assets/Fonts/`.
