include $(CLEAR_VARS)

SRC_DIR := ../../tests
INC_DIR := $(LOCAL_PATH)/../../include

LOCAL_C_INCLUDES += \
        $(INC_DIR) \

LOCAL_SRC_FILES := \
            $(SRC_DIR)/Tests.c \
            $(SRC_DIR)/TestBuffer.c \
            $(SRC_DIR)/TestCBuffer.c \
            $(SRC_DIR)/TestConcurrency.c \
            $(SRC_DIR)/TestMessageBox.c \
            $(SRC_DIR)/TestList.c \
            $(SRC_DIR)/TestPrefs.c \
            $(SRC_DIR)/TestTimer.c \
            $(SRC_DIR)/TestLog.c \
            $(SRC_DIR)/TestUtils.c \
            $(SRC_DIR)/TestStopwatch.c \
            $(SRC_DIR)/TestError.c \
            $(SRC_DIR)/TestBlockingQueue.c \
            $(SRC_DIR)/TestBufferQueue.c \

LOCAL_WHOLE_STATIC_LIBRARIES += libRingBuffer-static

LOCAL_SHARED_LIBRARIES := \
        liblog

LOCAL_LDLIBS += -llog

LOCAL_MODULE := libRingBuffer_tests

include $(BUILD_EXECUTABLE)
include $(CLEAR_VARS)

