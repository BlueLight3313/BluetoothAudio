# Linux build & run (Ubuntu 22.04) - v3.3 (no libasound)

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

## Identify your PCM device nodes
```sh
ls -l /dev/snd/
# typical:
# /dev/snd/pcmC0D0c  (capture)
# /dev/snd/pcmC0D0p  (playback)
```

## Run (foreground)
```sh
sudo ./bt_chatd --adapter-index 0 --tcp 127.0.0.1:3333 --audio linuxraw   --pcm-capture /dev/snd/pcmC0D0c --pcm-playback /dev/snd/pcmC0D0p
```

## Install as systemd service
```sh
sudo install -m 755 build/bt_chatd /usr/local/bin/bt_chatd
sudo install -m 644 service/linux/bt_chatd.service /etc/systemd/system/bt_chatd.service
sudo systemctl daemon-reload
sudo systemctl enable --now bt_chatd
sudo systemctl status bt_chatd
```
