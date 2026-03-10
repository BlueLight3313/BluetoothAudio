# Update Notes v3.2

## What was updated
### 1) Automation / state-driven workflow
Added:
- `src/state_machine.c/.h`
This thread executes a deterministic policy when `AUTO ON`:
- Ensure Bluetooth powered on
- If target set and not connected:
  - short SCAN burst to refresh device visibility
  - attempt PAIR (best-effort)
  - attempt CONNECT
- If disconnected later, it retries CONNECT automatically

### 2) Continuous MGMT event listener
Updated:
- `src/bt_mgmt.c`
Now it runs a listener thread that continuously receives MGMT events:
- DEVICE_FOUND -> updates scan cache
- DEVICE_CONNECTED / DEVICE_DISCONNECTED -> updates internal connected state
- PIN_CODE_REQUEST / USER_CONFIRM_REQUEST -> auto replies (PIN=0000 or accept confirm)

### 3) New control commands
Added:
- `SET_TARGET <MAC>`
- `AUTO ON|OFF`
- `STATE`

## Why
You requested the system should **automatically operate based on states**
between the device and the headset, rather than requiring manual command sequences.
v3.2 implements that using a simple state machine + event-driven state updates.

## Notes / limitations
- Some headsets require SDP advertising for HFP AG discovery. v3.2 still uses a fixed RFCOMM channel (8).
- On Android, MGMT and/or SCO may be blocked by SELinux; automation cannot bypass policy restrictions.
