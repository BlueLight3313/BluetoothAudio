# Workflow overview (v3.2)

## Threads / components
1) Control server thread (TCP or UNIX)
- Receives commands
- Updates 'intent' (AUTO, target MAC, number string)
- Provides STATUS/STATE output

2) MGMT thread (kernel events)
- Open HCI control channel (MGMT)
- Receives events:
  - device found -> scan list
  - connected/disconnected -> link state
  - pairing prompts -> auto replies

3) Automation state machine thread
- Runs periodically (1 Hz)
- If AUTO enabled:
  - drives power/scan/pair/connect toward the desired target state

4) HFP RFCOMM server thread
- Accepts headset RFCOMM connection
- Handles basic HFP AT commands
- Emits RING/+CLIP using your special "number" string

5) SCO audio bridge threads (TX+RX)
- TX: mic PCM -> CVSD -> SCO send
- RX: SCO recv -> CVSD -> speaker PCM

## End-to-end flow
User sets:
- target device: SET_TARGET <MAC>
- auto mode: AUTO ON

Daemon then keeps the BR/EDR link connected via MGMT.
When the headset establishes HFP (RFCOMM), AT signaling runs.
When call becomes active (e.g. headset sends ATA), SCO starts and PCM audio flows.
