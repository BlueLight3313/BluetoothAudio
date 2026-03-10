# Update Notes v3.3

## What changed
### 1) Removed ALSA user-space middleware (`libasound`)
- We no longer link to `libasound` and do not use `alsa/asoundlib.h`.
- Linux audio is implemented via **raw kernel PCM device nodes** using `<sound/asound.h>` ioctls:
  - capture: `/dev/snd/pcmC*D*c`
  - playback: `/dev/snd/pcmC*D*p`
- New file: `src/audio_linux_raw.c`
- Updated file: `src/audio.h` (backend is now `linuxraw`)

### 2) Service/daemon deployment examples
Added:
- `service/linux/bt_chatd.service` (systemd unit)
- `service/android/init.bt_chatd.rc` (Android init snippet)

## Why
You requested:
- no third-party middleware like ALSA library
- the project should run as an OS-managed service/daemon and react to events/state
