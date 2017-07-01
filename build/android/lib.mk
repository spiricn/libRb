
SRC_DIR := ../../source
INC_DIR := $(LOCAL_PATH)/../../include

LOCAL_SRC_FILES := \
            $(SRC_DIR)/RingBuffer.c \
            $(SRC_DIR)/ConcurrentRingBuffer.c \
            $(SRC_DIR)/MessageBox.c \
            $(SRC_DIR)/Stopwatch.c \
            $(SRC_DIR)/Log.c \
            $(SRC_DIR)/Common.c \
            $(SRC_DIR)/List.c \
            $(SRC_DIR)/IOStream.c \
            $(SRC_DIR)/Prefs.c \
            $(SRC_DIR)/PrefsBackend.c \
            $(SRC_DIR)/Utils.c \
            $(SRC_DIR)/FileStream.c \
            $(SRC_DIR)/LogPriv.c \
            $(SRC_DIR)/Timer.c \
            $(SRC_DIR)/ErrorPriv.c \
            $(SRC_DIR)/Vector.c \
            $(SRC_DIR)/ConsumerProducer.c \
            $(SRC_DIR)/BlockingQueue.c \
            $(SRC_DIR)/BufferQueue.c \

LOCAL_C_INCLUDES += \
        $(INC_DIR) \

LOCAL_SHARED_LIBRARIES := \
        liblog

LOCAL_CFLAGS:= -DANDROID

LOCAL_MODULE_TAGS := optional
