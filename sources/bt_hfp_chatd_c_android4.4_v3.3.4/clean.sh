#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
echo "Cleaning build artifacts under: $ROOT"
rm -rf "$ROOT/android_ndk/build" "$ROOT/android_ndk/out" "$ROOT/build"
find "$ROOT" \( -name "*.o" -o -name "*.obj" -o -name "*.a" -o -name "*.so" -o -name "CMakeCache.txt" -o -name "CMakeFiles" \) -print | while read -r p; do
  rm -rf "$p"
done
echo "Done."
