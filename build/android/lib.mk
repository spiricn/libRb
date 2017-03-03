
SRC_DIR := ../../source
INC_DIR := $(LOCAL_PATH)/../../include

LOCAL_SRC_FILES := \
			$(SRC_DIR)/RingBuffer.c \
			$(SRC_DIR)/ConcurrentRingBuffer.c \
			$(SRC_DIR)/MessageBox.c \
			$(SRC_DIR)/Array.c \
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
			$(SRC_DIR)/Timer.c
			
LOCAL_C_INCLUDES += \
		$(INC_DIR) \

LOCAL_SHARED_LIBRARIES := \
		liblog \
		libcutils \
		libutils \
		
LOCAL_CFLAGS:= -DANDROID

LOCAL_MODULE_TAGS := optional
