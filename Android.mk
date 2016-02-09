LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

ANDROID_NDK_ROOT=ndk

SRC_DIR := source
INC_DIR := $(LOCAL_PATH)/include

LOCAL_SRC_FILES := \
			$(SRC_DIR)/ConcurrentRingBuffer.c \
			$(SRC_DIR)/RingBuffer.c \
			$(SRC_DIR)/MessageBox.c \
					
LOCAL_C_INCLUDES += \
		$(INC_DIR) \

LOCAL_SHARED_LIBRARIES := \
		liblog \
		libcutils \
		libutils \

LOCAL_MODULE := libRingBuffer

LOCAL_CFLAGS:= -DANDROID

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

