# 125A Pitchator — Development V1

125A Pitchator is a planned **scale-aware MIDI pitch-bend sequencer** with
an editable 1–16-bar curve. This repository is an early development branch,
**not** a usable VST3 download or a finished product.

## Current implementation status

A native Steinberg VST3 processor/controller **prototype** is staged for the
v1.0.0 development line. The MIDI scheduler emits 14-bit Pitch Bend as
LegacyMIDICCOutEvent, forwards source notes, uses host PPQ/tempo or MIDI-Note
trigger, and serializes the curve and parameters in a versioned component state.
A VSTGUI graphical curve editor prototype has been added: point insertion,
dragging, removal, Snap, Step/Linear/Smooth, bar scrolling, graph zoom,
Clear Pattern and UI zoom 100/150%. Precise point entry is now available as
BAR.BEAT.TICK (0–959 ticks per beat) and signed decimal semitones (also
accepting German decimal comma), with 1/32 mouse snapping, editable
selection and variable pitch-axis scaling. It publishes validated snapshots
to the processor. **Windows x64 build, CTest and Steinberg validator are verified; Studio One GUI/MIDI interaction is still unverified**.

See [Native VST3 milestone](docs/NATIVE-VST3-MILESTONE.md) for engineering
limits and required tests. The newest verified prototype evidence is
[Windows Build #8](https://github.com/challanger2000/125A-Pitchator/actions/runs/38257844184):
**6/6 CTest + 47/47 Steinberg Validator + 125A Plugin Tester v0.2.9
28 PASS / 0 WARN / 0 FAIL**. Native editor lifecycle 5/5 cycles PASS;
isolated MIDI I/O event probe PASS. Build's Windows artifact ID
`11751885229`, QA diagnostics artifact ID `11750689704`.
Not yet Studio One/GUI mouse/zoom tested or Gumroad FINAL. Verified CI evidence: [Windows build #5](https://github.com/challanger2000/125A-Pitchator/actions/runs/38253823977),
commit `6a39f02`: **5/5 CTest PASS, 47/47 Steinberg Validator PASS**,
prototype artifact uploaded. This is *not* GUI click, Studio One routing,
125A Plugin Tester, or FINAL evidence.

## What the first development batch contains
- Tested-in-source C++17 musical curve engine, point editor data model,
  Step/Linear/Smooth interpolation and 14-bit MIDI Pitch Bend conversion.
- Twelve-key scale lookup, with target-only Scale Lock.
- Deterministic regression assertions (run with CMake/CTest).
- Binding V1 product requirements and end-to-end release QA matrix.

**Verified in native Windows Build #11:** 7/7 CTest (including VSTGUI
mouse input, every bottom command, numeric point fields, Scale Lock / Loop /
Bypass toggles and direct VST3 MIDI event output), 47/47 Steinberg Validator,
and 125A Plugin Tester 28 PASS / 0 WARN / 0 FAIL including Editor Lifecycle.
The 125A original master SVG is preserved and the ZIP contains the complete
VST3 bundle.

**Still open:** actual Studio One mouse/keyboard interaction and label redraw,
MIDI routing to a real synth, audible musical acceptance, 100/150% scale,
DAW save/reopen and full final human-host UI audit. DO NOT label FINAL.

## SDK
The verified latest stable Steinberg VST3 SDK on 2026-10-12 is
**3.8.1**, official tag **v3.8.1_build_84**.
That tag is recorded in the CMake project and is used by the existing
VST3 adapter (with its exact matching submodules).
No moving SDK master branch.

## Local developer core test
`cmake -S . -B build && cmake --build build && ctest --test-dir build --output-on-failure`

## Engineering workflow
Follow [125A-Engineering/START-HERE.md](https://github.com/challanger2000/125A-Engineering/blob/main/START-HERE.md).
Develop on the `v1.0.0` branch, leave `main` stable.
GitHub Actions workflow is **manual-only** to conserve budget.

Read [V1 contract](docs/V1_SPEC.md) and [QA traceability](docs/QA_TRACEABILITY.md).

## Artifact ZIP structure (fix applied after Build #8)

Build #8 used an incorrect artifact path that stripped the enclosing
`Pitchator.vst3` directory. The path is fixed; downloaded ZIPs from
Builds #9 and #11 have been verified to include
`Pitchator.vst3/Contents/...` at the archive root.

## Windows Build #9: Instrument classification and ZIP corrected

From 2026-10-12, the VST3 factory exports `Instrument|Synth` (same
classification convention as 125A Midiator), not `Fx|Tools`. This is
still a **MIDI-only** instrument utility: no audio synthesis is added.
The GitHub artifact ZIP includes the actual `Pitchator.vst3` bundle
directory at the archive root. Verified by independently downloading
artifact `11767334386` and checking the ZIP directory and embedded
`moduleinfo.json`. This build passes **6/6 CTest, 47/47 Steinberg
Validator and 28/28 125A Tester**. Actual Studio One routing and GUI
mouse interactions must still be tested in the DAW.

## Windows Build #11: direct VSTGUI interaction regression

Validated commit `a2230df10aa7fd35636d26d7fbb9590cdf34f84b`:
[CI logs](https://github.com/challanger2000/125A-Pitchator/actions/runs/38296952444).
- **7/7 CTest**, **47/47 Steinberg Validator**, **28/28 125A QA PASS**.
- Curve view now uses VSTGUI parent-coordinate mouse positions, without
  accidentally double-transforming them.
- Scale Lock, Loop and Bypass now have persistent `kOnOffStyle` buttons,
  explicit MIDI-parameter Begin/Perform/End host edits, visible ON/OFF titles,
  and sync after automation and saved-state restore. The VSTGUI default
  `kKickStyle` caused these controls to reset to OFF immediately.
- All eight bottom commands get actual VSTGUI MouseDown/MouseUp regression
  coverage; both exact-value fields get controller-route tests.
- Graph explicitly selects its font on redraw to address intermittent
  disappearing graph labels; physical Studio One redraw still needs checking.
- New test artifact ID `11768729087`; full bundle in ZIP.

[125A QA traceability](docs/QA_TRACEABILITY.md) contains host/test limits.
