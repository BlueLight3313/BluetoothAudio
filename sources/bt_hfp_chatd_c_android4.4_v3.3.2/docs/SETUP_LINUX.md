# Linux build & run (Ubuntu 22.04) - v3.3.1 (no libasound)

## Build deps
```sh
sudo apt update
sudo apt install -y build-essential cmake pkg-config netcat-openbsd
```

## Build
```sh
mkdir -p build && cd build
cmake ..
cmake --build . -j
```

## Identify PCM devices
```sh
ls -l /dev/snd/
# capture:  /dev/snd/pcmC0D0c
# playback: /dev/snd/pcmC0D0p
```

## Run
```sh
sudo ./bt_chatd --adapter-index 0 --tcp 127.0.0.1:3333 --audio linuxraw   --pcm-capture /dev/snd/pcmC0D0c --pcm-playback /dev/snd/pcmC0D0p
```
