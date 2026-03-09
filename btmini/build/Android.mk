LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := btmini

LOCAL_SRC_FILES := \
    ../src/main.c \
    ../src/hci.c \
    ../src/util.c

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/../include

LOCAL_LDLIBS := -llog

include $(BUILD_EXECUTABLE)