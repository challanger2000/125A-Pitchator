# 125A Pitchator — Development V1

125A Pitchator is a planned **scale-aware MIDI pitch-bend sequencer** with
an editable 1–16-bar curve. This repository is an early development branch,
**not** a usable VST3 download or a finished product.

## What the first development batch contains
- Tested-in-source C++17 musical curve engine, point editor data model,
  Step/Linear/Smooth interpolation and 14-bit MIDI Pitch Bend conversion.
- Twelve-key scale lookup, with target-only Scale Lock.
- Deterministic regression assertions (run with CMake/CTest).
- Binding V1 product requirements and end-to-end release QA matrix.

**Still to develop:** MIDI input/output VST3 processor and controller,
realtime event generation, complete graphical editor, state recall, hosting
validation and Studio One acceptance. No successful build or DAW test is
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
