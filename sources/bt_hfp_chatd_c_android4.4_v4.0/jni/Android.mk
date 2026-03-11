LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := bt_hfp_chatd_map_v40
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_SRC_FILES := \
    src/msg_store.c \
    src/map_sdp.c \
    src/map_mns.c \
    src/map_mse.c \
    src/map_integration_example.c
LOCAL_CFLAGS := -Wall -Wextra -std=c99
include $(BUILD_SHARED_LIBRARY)
