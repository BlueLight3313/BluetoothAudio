# Linux build & run (Ubuntu 22.04)

## Build deps
```sh
sudo apt update
sudo apt install -y build-essential cmake pkg-config libasound2-dev netcat-openbsd
```

## Build
```sh
mkdir -p build && cd build
cmake ..
cmake --build . -j
```

## Run
```sh
sudo ./bt_chatd --adapter-index 0 --tcp 127.0.0.1:3333 --audio alsa
```

## Automation example
```sh
# set target and enable automation
echo "SET_TARGET AA:BB:CC:DD:EE:FF" | nc 127.0.0.1 3333
echo "AUTO ON" | nc 127.0.0.1 3333
echo "STATE" | nc 127.0.0.1 3333
```
