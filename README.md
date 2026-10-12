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
Clear Pattern and UI zoom 100/150%. It publishes validated snapshots
to the processor. **Real host interaction and build are not yet verified**.

See [Native VST3 milestone](docs/NATIVE-VST3-MILESTONE.md) for engineering
limits and required tests. No successful CI build, validator, GUI or DAW PASS
is claimed in this commit.

## What the first development batch contains
- Tested-in-source C++17 musical curve engine, point editor data model,
  Step/Linear/Smooth interpolation and 14-bit MIDI Pitch Bend conversion.
- Twelve-key scale lookup, with target-only Scale Lock.
- Deterministic regression assertions (run with CMake/CTest).
- Binding V1 product requirements and end-to-end release QA matrix.

**Still open:** numeric point-position/pitch editor, MIDI event-list integration
tests, GUI click/lifecycle tests, Steinberg Validator, real Studio One acceptance,
and complete release QA. The original 125A master SVG is included but not
substituted with a font recreation or an unverified raster conversion. No successful build or DAW test is
claimed until independently observed.

## SDK
The verified latest stable Steinberg VST3 SDK on 2026-10-12 is
**3.8.1**, official tag **v3.8.1_build_84**.
That tag is recorded in the CMake project and must be used in the upcoming
VST3 adapter (with its exact matching submodules).
No moving SDK master branch.

## Local developer core test
`cmake -S . -B build && cmake --build build && ctest --test-dir build --output-on-failure`

## Engineering workflow
Follow [125A-Engineering/START-HERE.md](https://github.com/challanger2000/125A-Engineering/blob/main/START-HERE.md).
Develop on the `v1.0.0` branch, leave `main` stable.
GitHub Actions workflow is **manual-only** to conserve budget.

Read [V1 contract](docs/V1_SPEC.md) and [QA traceability](docs/QA_TRACEABILITY.md).
