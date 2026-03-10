#!/usr/bin/env bash
set -euo pipefail
ABI="${1:-arm64-v8a}"
if [[ -z "${ANDROID_NDK:-}" ]]; then
  echo "Set ANDROID_NDK=/path/to/android-ndk" >&2
  exit 2
fi
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT="$ROOT/android_ndk/out/$ABI"
BUILD="$ROOT/android_ndk/build/$ABI"
mkdir -p "$OUT" "$BUILD"
cmake -S "$ROOT" -B "$BUILD" \
  -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK/build/cmake/android.toolchain.cmake" \
  -DANDROID_ABI="$ABI" \
  -DANDROID_PLATFORM=android-21 \
  -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD" -j
cp -v "$BUILD/bt_chatd" "$OUT/bt_chatd"
echo "Output: $OUT/bt_chatd"
