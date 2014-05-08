LOCAL_BUILD_PATH := $(call my-dir)

include $(CLEAR_VARS)

#include $(LOCAL_BUILD_PATH)/llvm/llvm-3.1.src/Android.mk
include $(LOCAL_BUILD_PATH)/neondetect/neondetect.mk

include $(LOCAL_BUILD_PATH)/dsoidneon.mk
include $(LOCAL_BUILD_PATH)/dsoidv7.mk
