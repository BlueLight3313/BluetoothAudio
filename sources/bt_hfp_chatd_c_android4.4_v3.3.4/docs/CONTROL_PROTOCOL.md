# Control protocol (v3.2)

Line-based request -> single-line reply.

## Status
- STATUS        (brief)
- STATE         (detailed internal state)

## Automation
- SET_TARGET <MAC>
- AUTO ON|OFF

## Bluetooth management (manual override)
- POWER ON|OFF
- SCAN ON|OFF
- SCAN_RESULTS
- PAIR <MAC>
- CONNECT <MAC>
- DISCONNECT <MAC>

## Caller ID special string
- SET_NUMBER <string>

## Call / audio
- DIAL <MAC>
- HANGUP
