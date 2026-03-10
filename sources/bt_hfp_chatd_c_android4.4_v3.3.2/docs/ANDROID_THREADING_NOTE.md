# Android threading note (v3.3.2)

Android **does support threads** (POSIX pthread APIs) via bionic libc.

But Android does **not** ship a separate `libpthread.so`, so linking with `-lpthread` fails:
- `ld.lld: unable to find library -lpthread`

Fix in v3.3.2:
- Linux: link pthread
- Android: do not link -lpthread
