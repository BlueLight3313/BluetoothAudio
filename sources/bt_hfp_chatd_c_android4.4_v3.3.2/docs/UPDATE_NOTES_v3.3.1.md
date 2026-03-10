# Build fix (v3.3.1)

## Why your NDK build failed
Your compiler error:
- `fatal error: 'third_party/linux_uapi_bt/bluetooth.h' file not found`

Reason:
- The source uses includes like:
  - `#include "third_party/linux_uapi_bt/bluetooth.h"`
- But the CMake include dirs in v3.2 were:
  - `src`
  - `third_party/linux_uapi_bt`
So the compiler can find `"bluetooth.h"`, but NOT `"third_party/linux_uapi_bt/bluetooth.h"` unless the **project root** is on the include path.

## What changed in v3.3.1
- Updated `CMakeLists.txt` to include the **project root** as an include directory:
  - `target_include_directories(bt_chatd PRIVATE ${CMAKE_SOURCE_DIR})`

That makes path-style includes work for both Linux and Android builds.

## Android API level note
NDK r27+ does not support `android-19` and automatically bumps to 21.
If you truly must target Android 4.4 (API 19) you need an older NDK (e.g., r21e).
