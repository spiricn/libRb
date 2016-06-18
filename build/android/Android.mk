LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

SRC_DIR := ../../source
INC_DIR := $(LOCAL_PATH)/../../include

LOCAL_SRC_FILES := \
			$(SRC_DIR)/RingBuffer.c \
			$(SRC_DIR)/ConcurrentRingBuffer.c \
			$(SRC_DIR)/MessageBox.c \
			$(SRC_DIR)/Array.c \
			$(SRC_DIR)/Stopwatch.c \
			$(SRC_DIR)/Log.c \
					
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

SRC_DIR := ../../tests
INC_DIR := $(LOCAL_PATH)/../../include

LOCAL_SRC_FILES := \
			$(SRC_DIR)/Tests.c \
			$(SRC_DIR)/TestArray.c \
			$(SRC_DIR)/TestBuffer.c \
			$(SRC_DIR)/TestCBuffer.c \
			$(SRC_DIR)/TestConcurrency.c \
			$(SRC_DIR)/TestMessageBox.c \

LOCAL_C_INCLUDES += \
		$(INC_DIR) \

LOCAL_MODULE := libRingBuffer_tests

LOCAL_SHARED_LIBRARIES += libRingBuffer

include $(BUILD_EXECUTABLE)
include $(CLEAR_VARS)