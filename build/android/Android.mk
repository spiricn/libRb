LOCAL_PATH := $(call my-dir)

# Build a shared library
include $(CLEAR_VARS)
include $(LOCAL_PATH)/lib.mk
include $(BUILD_SHARED_LIBRARY)

# Build a static library
include $(CLEAR_VARS)
include $(LOCAL_PATH)/lib.mk
include $(BUILD_STATIC_LIBRARY)

# Build test binaries
include $(LOCAL_PATH)/tests.mk