# Android NDK build & run - v3.3

Android audio uses OpenSL ES (system API). No ALSA library is used.

## Build
```sh
cd android_ndk
export ANDROID_NDK=/path/to/android-ndk
./build.sh arm64-v8a
```

## Push/run (testing)
```sh
adb push android_ndk/out/arm64-v8a/bt_chatd /data/local/tmp/bt_chatd
adb shell chmod 755 /data/local/tmp/bt_chatd
adb shell su -c "/data/local/tmp/bt_chatd --adapter-index 0 --tcp 127.0.0.1:3333 --audio opensles"
```

## Run as init service (product image)
See: service/android/init.bt_chatd.rc
(SELinux policy may be required for MGMT/SCO on Android 12.)
