cmake_minimum_required(VERSION 3.10)
project(bt_chatd_c C)
set(CMAKE_C_STANDARD 99)
add_executable(bt_chatd
  src/main.c
  src/bt_util.c
  src/cvsd.c
  src/audio_linux_raw.c
  src/audio_opensles.c
  src/sco_bridge.c
  src/hfp_ag.c
  src/ctrl_tcp.c
  src/ctrl_unix.c
  src/bt_mgmt.c
  src/state_machine.c
)
target_include_directories(bt_chatd PRIVATE src third_party/linux_uapi_bt)
target_link_libraries(bt_chatd PRIVATE pthread)
if(NOT ANDROID)
endif()
if(ANDROID)
  target_link_libraries(bt_chatd PRIVATE OpenSLES)
endif()