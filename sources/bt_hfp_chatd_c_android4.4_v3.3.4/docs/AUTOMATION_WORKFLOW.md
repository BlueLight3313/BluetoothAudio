# Automation workflow (v3.2)

## Recommended usage
1) Start daemon
2) Set target headset MAC
3) Enable automation
4) Watch STATE until connected

Commands:
- SET_TARGET AA:BB:CC:DD:EE:FF
- AUTO ON
- STATE

Disable:
- AUTO OFF

## Example (TCP)
```sh
echo "SET_TARGET AA:BB:CC:DD:EE:FF" | nc 127.0.0.1 3333
echo "AUTO ON" | nc 127.0.0.1 3333
echo "STATE" | nc 127.0.0.1 3333
```
