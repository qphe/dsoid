LOCAL_PATH := $(call my-dir)

MY_LOCAL_PATH := $(LOCAL_PATH)

include $(CLEAR_VARS)
#$(call import-add-path, /cygdrive/c/AndroidNDK/android-ndk-r8c/sources/android/cpufeatures)

LOCAL_MODULE    		:= 	neondetect
#NDK_MODULE_PATH := /cygdrive/c/AndroidNDK/android-ndk-r8c/sources/android/cpufeatures
LOCAL_SRC_FILES			:= 	neondetect.cpp
LOCAL_ARM_NEON 			:= false
LOCAL_CFLAGS			:= -fexceptions
LOCAL_STATIC_LIBRARIES := cpufeatures

include $(BUILD_SHARED_LIBRARY)


$(call import-module,cpufeatures)