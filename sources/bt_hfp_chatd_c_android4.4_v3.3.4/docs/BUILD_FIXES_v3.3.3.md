# Build fixes v3.3.3

## Fixed issues seen in your log
1) Multiple definition of audio_* functions
- Cause: both `audio_linux_raw.c` and `audio_opensles.c` were compiled into the same binary.
- Fix: CMake now selects **one** audio backend:
  - ANDROID -> audio_opensles.c
  - non-ANDROID -> audio_linux_raw.c

2) Multiple definition of ctrl_server_stop
- Cause: both ctrl_tcp.c and ctrl_unix.c defined the same symbol.
- Fix: replaced them with a single unified implementation: `src/ctrl_server.c`.

3) Added build clean step
- android_ndk/build.sh now deletes the per-ABI build directory before configuring.

## Threading note
Android does support pthreads (in bionic libc). Do not link `-lpthread` on Android.
