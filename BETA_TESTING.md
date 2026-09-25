# Bowie beta testing

Thank you for testing Bowie. This beta contains an unsigned standalone app and
VST3 instrument for Windows and macOS.

## What is new in 0.5.0

- The body palette now includes Harp, Harpsichord, Oboe, Farfisa, and Marimba.
- Voices 1, 2, and 3 have independent ADSR envelopes, selected with the compact
  `ENV 1 / 2 / 3` switch.
- The revised Fanfare preset layers Farfisa, Oboe, and Trumpet with a distinct
  envelope for each body.

## Install

### Windows

- The standalone `Bowie.exe` can run from any writable folder.
- For a DAW, copy `Bowie.vst3` to `C:\Program Files\Common Files\VST3`, then
  rescan plug-ins in the DAW.
- Windows SmartScreen may warn because the beta is not code-signed.

The standalone prefers a native ASIO driver when one is installed, then falls
back to shared Windows Audio. Audio/MIDI settings remain available from the
standalone application's menu.

### macOS

- Drag `Bowie.app` to Applications if you want to use the standalone.
- For a DAW, copy `Bowie.vst3` to `~/Library/Audio/Plug-Ins/VST3`, then rescan
  plug-ins in the DAW.
- The beta is not notarized. On first launch, control-click the app and choose
  **Open**, then confirm the prompt.

## Useful feedback

Please include:

- Windows or macOS version and CPU type.
- Standalone or VST3, plus DAW and version when applicable.
- Audio interface and driver type.
- What you played or changed immediately before the problem.
- Whether the issue repeats after restarting Bowie.
- A screenshot or short audio example when it helps.

Musical impressions are just as useful as bugs: favourite presets, awkward
ranges, unexpected level jumps, and combinations that feel especially alive.
