ifeq ($(TARGET_BOARD_PLATFORM),omap4)


#
# MotHDR Wrapper
#

#LOCAL_PATH:= $(call my-dir)
#include $(CLEAR_VARS)

#######

#
# libcamera
#

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    CameraHalM.cpp \
    CameraHal.cpp \
    CameraHalUtilClasses.cpp \
    AppCallbackNotifier.cpp \
    MemoryManager.cpp	\
    ANativeWindowDisplayAdapter.cpp \
    CameraProperties.cpp \
    TICameraParameters.cpp \

#    $(LOCAL_PATH)/../inc/HDRInterface \
LOCAL_C_INCLUDES += \
    bionic/libc/include \
    frameworks/base/include/ui \
    frameworks/base/include/utils \
    $(LOCAL_PATH)/inc \
    $(LOCAL_PATH)/../hwc \
    $(LOCAL_PATH)/../libtiutils \
    $(LOCAL_PATH)/../../omx/ducati/domx/system/omx_core/inc \
    $(LOCAL_PATH)/../../omx/ducati/domx/system/mm_osal/inc \
    $(LOCAL_PATH)/../../tiler \
    $(LOCAL_PATH)/../../syslink/ipc-listener \
    external/libxml2/include \
    external/icu4c/common \

LOCAL_SHARED_LIBRARIES:= \
    libdl \
    libui \
    libbinder \
    libutils \
    libcutils \
    libtiutils \
    libtimemmgr \
    libicuuc \
    libcamera_client \
    libsyslink_ipc_listener \
    libhdr_interface \

LOCAL_CFLAGS += -fno-short-enums -DCOPY_IMAGE_BUFFER -DTARGET_OMAP4 -mfpu=neon

LOCAL_MODULE:= libcamera
LOCAL_MODULE_TAGS:= optional

include $(BUILD_SHARED_LIBRARY)

#######

#
# OMX Camera Adapter 
#

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    BaseCameraAdapter.cpp \
    OMXCameraAdapter/OMXCap.cpp \
    OMXCameraAdapter/OMXCameraAdapter.cpp \

LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/inc/ \
    $(LOCAL_PATH)/inc/OMXCameraAdapter \
    $(LOCAL_PATH)/../hwc \
    $(LOCAL_PATH)/../libtiutils \
    external/icu4c/common \

LOCAL_SHARED_LIBRARIES:= \
    libdl \
    libui \
    libbinder \
    libutils \
    libcutils \
    libtiutils \
    libOMX_CoreOsal \
    libOMX_Core \
    libsysmgr \
    librcm \
    libipc \
    libcamera \
    libicuuc \
    libcamera_client \
    libomx_rpc \
    libhdr_interface \
    libhardware_legacy \

LOCAL_C_INCLUDES += \
    bionic/libc/include \
    frameworks/base/include/ui \
    frameworks/base/include/utils \
    $(LOCAL_PATH)/../libtiutils \
    $(LOCAL_PATH)/../../omx/ducati/domx/system/omx_core/inc \
    $(LOCAL_PATH)/../../omx/ducati/domx/system/mm_osal/inc \
    external/libxml2/include \

LOCAL_CFLAGS += -fno-short-enums -DTARGET_OMAP4

LOCAL_MODULE:= libomxcameraadapter
LOCAL_MODULE_TAGS:= optional

include $(BUILD_SHARED_LIBRARY)

#######

#
# OMX Camera HAL 
#

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    CameraHal_Module.cpp

LOCAL_C_INCLUDES += \
    bionic/libc/include \
    frameworks/base/include/ui \
    frameworks/base/include/utils \
    $(LOCAL_PATH)/inc \
    $(LOCAL_PATH)/../libtiutils \
    $(LOCAL_PATH)/../../omx/ducati/domx/system/omx_core/inc \
    $(LOCAL_PATH)/../../omx/ducati/domx/system/mm_osal/inc \
    $(LOCAL_PATH)/../../../../external/libxml2/include \
    $(LOCAL_PATH)/../../tiler \
    $(LOCAL_PATH)/../../syslink/ipc-listener \
    $(LOCAL_PATH)/../hwc \
    external/icu4c/common \


LOCAL_SHARED_LIBRARIES:= \
    libcamera \
    libomxcameraadapter \
    libdl \
    libui \
    libbinder \
    libutils \
    libcutils \
    libtiutils \
    libOMX_CoreOsal \
    libOMX_Core \
    libsysmgr \
    librcm \
    libipc \
    libcamera \
    libicuuc \
    libcamera_client \
    libomx_rpc \
    libhdr_interface \
    libhardware_legacy \

LOCAL_STATIC_LIBRARIES:= \
    libxml2 \

LOCAL_CFLAGS += -fno-short-enums -DCOPY_IMAGE_BUFFER -DTARGET_OMAP4 -mfpu=neon

LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
LOCAL_MODULE:= camera.$(TARGET_BOARD_PLATFORM)
LOCAL_MODULE_TAGS:= optional

include $(BUILD_HEAPTRACKED_SHARED_LIBRARY)

endif
