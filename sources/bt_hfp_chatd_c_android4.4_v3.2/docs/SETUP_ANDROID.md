# Android NDK build & run

## Build (Linux host)
```sh
cd android_ndk
export ANDROID_NDK=/path/to/android-ndk
./build.sh arm64-v8a
```

## Push & run
```sh
adb push android_ndk/out/arm64-v8a/bt_chatd /data/local/tmp/bt_chatd
adb shell chmod 755 /data/local/tmp/bt_chatd
adb shell su -c "/data/local/tmp/bt_chatd --adapter-index 0 --tcp 127.0.0.1:3333 --audio opensles"
```

## Automation example
```sh
adb shell "echo 'SET_TARGET AA:BB:CC:DD:EE:FF' | nc 127.0.0.1 3333"
adb shell "echo 'AUTO ON' | nc 127.0.0.1 3333"
adb shell "echo 'STATE' | nc 127.0.0.1 3333"
```

Note: MGMT may be blocked by SELinux on some Android builds.
