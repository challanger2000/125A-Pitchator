# 125A Pitchator V1 — QA control & requirement traceability

Legend: **CORE TEST PASS** = locally defined deterministic assertions only;
**NOT VERIFIED** = no valid end-to-end evidence yet. Never equate core tests
to VST3 validator, 125A tester or Studio One acceptance.

| Requirement / GUI control | Processor / action | MIDI or state consequence | Evidence/status |
| --- | --- | --- | --- |
| Curve point insert / drag / delete | Pattern::upsert/erase | Target pitch recalculation | Core test defined; host NOT VERIFIED |
| Clear Pattern | Pattern::clear | Neutral 0st curve + MIDI reset TBD | Core test defined; host NOT VERIFIED |
| Taktzahl 1–16 | Pattern::setBars | Duration in MIDI PPQ ticks | Core test defined; host NOT VERIFIED |
| Key / Scale | scale masks + snapTarget | Scale-correct destination | Core test defined; host NOT VERIFIED |
| Semitone / Scale Lock | value() | Literal / snapped destination | Core test defined; host NOT VERIFIED |
| Step / Linear / Smooth | interpolate() | Slope and step of bend | Core test defined; host NOT VERIFIED |
| Bend Range | bend14() | MIDI 14-bit center and endpoints | Core test defined; host NOT VERIFIED |
| Grid / Snap / Scroll / Zoom | GUI TBD | Editor pitch-point time/position | NOT VERIFIED |
| HOST SYNC | hostTick() | PPQ-to-tick phase | Core test defined; host NOT VERIFIED |
| NOTE TRIGGER | event scheduler TBD | Sample-accurate restart | NOT VERIFIED |
| Loop / Once | event scheduler TBD | Bounded 1–16-bar playback | NOT VERIFIED |
| MIDI pass-through | VST3 processor TBD | Identical note on/off/order | NOT VERIFIED |
| Standard MIDI Pitch Bend output | VST3 processor TBD | LegacyMIDICCOutEvent 14-bit | NOT VERIFIED |
| Bypass / Panic / Stop | processor TBD | Center pitch + cleanup | NOT VERIFIED |
| GUI 100/150% & mouse hitboxes | VSTGUI TBD | Editor and interaction | NOT VERIFIED |
| Save/Load / Automation / Reopen | VST3 state TBD | Identical restored curve | NOT VERIFIED |
| Validator / Plugin Tester | Release QA | Host compliance | NOT VERIFIED |
| Studio One note, routing, bend | DAW interaction | Musical acceptance | NOT VERIFIED |

All GUI controls require a complete UI tag -> input -> controller -> parameter /
command -> processor -> measured MIDI -> visual feedback -> automation & recall
chain before V1 can be called finished.
