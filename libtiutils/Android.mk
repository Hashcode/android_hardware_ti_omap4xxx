################################################

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

LOCAL_SRC_FILES:= \
    MessageQueue.cpp \
    Semaphore.cpp \
    ErrorUtils.cpp
    
LOCAL_SHARED_LIBRARIES:= \
    libdl \
    libui \
    libbinder \
    libutils \
    libcutils

LOCAL_C_INCLUDES += \
    $(TOP)/frameworks/base/include/utils \
    $(TOP)/bionic/libc/include \
    $(TOP)/hardware/ti/omap4xxx/domx/system/omx_core/inc \
    $(TOP)/hardware/ti/omap4xxx/domx/system/mm_osal/inc
	
LOCAL_CFLAGS += -fno-short-enums 

# LOCAL_CFLAGS +=

LOCAL_MODULE:= libtiutils
LOCAL_MODULE_TAGS:= optional

include $(BUILD_HEAPTRACKED_SHARED_LIBRARY)
