# Android NDK build & run - v3.3.1

## NDK note
If you are using NDK r27+, `android-19` is unsupported and the toolchain uses API 21.
To build specifically for API 19 (Android 4.4), use an older NDK (e.g., r21e).

## Build
```sh
cd android_ndk
export ANDROID_NDK=/path/to/android-ndk
./build.sh arm64-v8a
```
