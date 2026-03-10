@echo off

set NDK=C:\Android\android-ndk-r27d
set TOOLCHAIN=%NDK%\toolchains\llvm\prebuilt\windows-x86_64\bin

%TOOLCHAIN%\aarch64-linux-android21-clang hello.c -o hello