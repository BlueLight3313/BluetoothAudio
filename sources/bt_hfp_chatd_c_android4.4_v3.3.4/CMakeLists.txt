cmake_minimum_required(VERSION 3.10)
project(bt_chatd_c C)
set(CMAKE_C_STANDARD 99)
add_executable(bt_chatd
  src/main.c
  src/bt_util.c
  src/cvsd.c
  src/sco_bridge.c
  src/hfp_ag.c
  src/ctrl_server.c
  src/bt_mgmt.c
  src/state_machine.c
)
target_include_directories(bt_chatd PRIVATE ${CMAKE_SOURCE_DIR} src third_party/linux_uapi_bt)
if(NOT ANDROID)
endif()
if(ANDROID)
endif()

# Audio backend selection
if(ANDROID)
  target_sources(bt_chatd PRIVATE src/audio_opensles.c)
  target_link_libraries(bt_chatd PRIVATE OpenSLES)
else()
  target_sources(bt_chatd PRIVATE src/audio_linux_raw.c)
  target_link_libraries(bt_chatd PRIVATE pthread)
endif()
