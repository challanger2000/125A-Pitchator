# 125A Pitchator V1 — binding product contract

**Project state:** architecture and MIDI pitch-curve core in development. NO release claim.

## Product
Windows x64 Steinberg VST3 MIDI effect, not an instrument and not an audio processor.
Input MIDI notes pass through; output contains standard MIDI channel pitch-bend (14-bit).
No MPE and no independent per-note pitch for simultaneous notes on one channel.
A downstream synth must be configured to match the plugin's bend range.

## Timeline & editor
1–16 bars. V1 musical grid: 4/4, up to 1/32 resolution, project/transport synchronization.
Every point specifies (tick, pitch offset in semitones, interpolation to next point).
Point 0 is mandatory at 0 st; right/left click, drag, delete, exact numeric editing,
scroll, zoom, clear/reset, bypass and 100/150% interface zoom are required GUI tasks.
Transitions Step, Linear, Smooth. User example: 0 → +3 st → +1 st → 0.
Points are editable independently of tempo and sample rate.
The curve may loop, or play once. Modes: HOST SYNC and NOTE TRIGGER.
In NOTE TRIGGER, an incoming Note-On starts at phase 0; actual sample offsets matter.
The source MIDI note is never rewritten to implement bending.

## Tonal semantics
SEMITONE mode: offsets are literal, e.g. +3 then -2 RELATIVE to current
destination gives +1 relative to the originally played note.
SCALE LOCK: target pitches snap to notes belonging to KEY/SCALE, with deterministic
tie-breaking in travel direction. 0 st stays 0, even when a user's played
source note is outside the configured key. A continuous glide may pass through
other pitches; SCALE LOCK constrains the destinations only.
KEY: 12 pitch classes. Scales: chromatic, major, natural/harmonic minor,
major/minor pentatonic, dorian, phrygian, lydian, mixolydian, locrian.
BEND RANGE must match the receiving synth and the 14-bit signed/asymmetric
mapping must preserve center 8192 and endpoints 0 / 16383.

## Host and lifecycle
Forward original NoteOn/NoteOff, velocity, channel, note ID and sample offset.
Honor host transport PPQ and stop/restart, same-block automation, offline rendering,
project recall, bypass and panic/reset events.
Do not fabricate an output MIDI pitch bend when processor is bypassed.
Always return bend to center when stopping, bypassing, clearing and ending
NOTE TRIGGER, with defined overlap/channel behavior.
Generate bounded events, realtime-safe; no heap/locks/files/formatting in callback.
Tests must check actual output events, not just curve evaluation.

## Version and engineering rules
Use latest verified stable SDK 3.8.1 tag v3.8.1_build_84 (2026-08-11),
C++17, official Steinberg VST3 interfaces and VSTGUI.
Product static design + approved 125A-Branding master before custom VSTGUI code.
Do not develop on main. Bundle commits/CI. No false FULL AUDIT claims.
Final release: Steinberg Validator, 125A Tester, editor lifecycle, exact event
routing, Studio One host playback and complete UI/parameter/end-to-end audit.

## Historical scope statement
The first commit did not contain the VST3 wrapper, GUI or state serialization.
These components have since been implemented and Windows-compiled in the
development branch. The hostless event-list tests and validator are green,
but practical Studio One/GUI lifecycle/125A Tester evidence is still missing.

## Numeric editing contract (V1 extension)
Use BAR.BEAT.TICK (e.g. 2.3.120; ticks 0–959 inside each beat);
960 PPQ/quarter, 3840 PPQ/bar. User-facing values start at bar 1,
beat 1, tick 000. The curve stores exact ticks and the mouse Snap
supports 1/32 notes (120 ticks), without rounding typed positions.
Type pitch as signed semitones with decimal dot or German decimal
comma; all edits reject invalid/out-of-range values, cannot move
the immutable point 0 or silently replace a point at an occupied time.
