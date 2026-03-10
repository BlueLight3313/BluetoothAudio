# bt_hfp_chatd_c v3.2 (pure C, state-driven automation)

Variant: Android 4.4 (default UNIX /dev/socket/bt_chatd)

## What this project is
A native Bluetooth voice-chat daemon that:
- Scans, pairs, connects, and reconnects to a headset (**device-controlled**)
- Speaks a minimal HFP-like AT protocol over RFCOMM
- Bridges audio over SCO using CVSD (PCM16 mono 8 kHz)

## Major update in v3.2
Earlier versions were mostly *manual command driven* (you had to type SCAN/PAIR/CONNECT).
v3.2 adds a **state machine** so the daemon can operate automatically based on state:

1) You set the *target headset* MAC with `SET_TARGET <MAC>`
2) You enable automation with `AUTO ON`
3) The daemon automatically:
   - powers on Bluetooth
   - performs short discovery bursts
   - attempts to pair (best-effort)
   - connects and keeps reconnecting if the link drops

Manual commands still exist and can override automation.

## Dependencies
- Bluetooth management uses Linux kernel **MGMT** API (no bluetoothd, no D-Bus)
- Audio:
  - Linux: ALSA (build-time dependency `libasound2-dev`)
  - Android: OpenSL ES (no extra deps)

## Where to read what changed and why
- docs/UPDATE_NOTES_v3.2.md
- docs/WORKFLOW_OVERVIEW.md
- docs/AUTOMATION_WORKFLOW.md


## v3.3.1 build fix
See docs/UPDATE_NOTES_v3.3.1.md


## v3.3.3
See docs/BUILD_FIXES_v3.3.3.md


## v3.3.4
- Fixed OpenSLES linking on Android
- Simplified include paths
- Added clean.sh
