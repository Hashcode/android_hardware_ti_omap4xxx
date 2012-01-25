LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

# prerequisites for building libcamera
file := $(LIBCAMERA_INTERMEDIATES_PREREQS)/libhdr_interface.so
$(file) : vendor/motorola/solana/proprietary/libhdr_interface.so
	@echo "Copy libhdr_interface.so -> $@"
	@mkdir -p $(dir $@)
	@rm -rf $@
	$(hide) cp -a vendor/motorola/solana/proprietary/libhdr_interface.so $@

LOCAL_SRC_FILES:= \
    CameraHalM.cpp \

#    CameraHal.cpp \
#    CameraHalUtilClasses.cpp \
#    AppCallbackNotifier.cpp \
#    MemoryManager.cpp	\
#    ANativeWindowDisplayAdapter.cpp \
#    CameraProperties.cpp \
#    TICameraParameters.cpp

#    $(LOCAL_PATH)/../inc/HDRInterface \

LOCAL_C_INCLUDES += \
    $(TOP)/hardware/ti/omap4xxx/camera/inc \
    $(TOP)/hardware/ti/omap4xxx/domx/system/mm_osal/inc \
    $(TOP)/hardware/ti/omap4xxx/domx/system/omx_core/inc \
    $(TOP)/hardware/ti/omap4xxx/hwc \
    $(TOP)/hardware/ti/omap4xxx/libtiutils \
    $(TOP)/hardware/ti/tiler \
    $(TOP)/hardware/ti/syslink/ipc-listener \
    $(TOP)/external/libxml2/include \
    $(TOP)/external/icu4c/common \
    $(TOP)/frameworks/base/include/utils \
    $(TOP)/frameworks/base/services/camera/libcameraservice \


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
    libhdr_interface \
    libsyslink_ipc_listener \

LOCAL_STATIC_LIBRARIES:= \
    libxml2

LOCAL_CFLAGS += -fno-short-enums -DCOPY_IMAGE_BUFFER -DTARGET_OMAP4 -mfpu=neon

LOCAL_MODULE:= libcamera
LOCAL_MODULE_TAGS:= optional

include $(BUILD_SHARED_LIBRARY)

ifeq (0,1)
#
# MotHDR Wrapper
#

#LOCAL_PATH:= $(call my-dir)
#include $(CLEAR_VARS)

#######

#
# libcamera
#


#######

#
# OMX Camera Adapter 
#

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    BaseCameraAdapter.cpp \
    OMXCameraAdapter/OMXCap.cpp \
    OMXCameraAdapter/OMXCameraAdapter.cpp \

LOCAL_C_INCLUDES += \
    bionic/libc/include \
    frameworks/base/include/ui \
    frameworks/base/include/utils \
    hardware/ti/omap4xxx/camera/inc/ \
    hardware/ti/omap4xxx/camera/inc/OMXCameraAdapter \
    hardware/ti/omap4xxx/hwc \
    hardware/ti/omap4xxx/libtiutils \
    hardware/ti/omap4xxx/libtiutils \
    hardware/ti/omx/ducati/domx/system/omx_core/inc \
    hardware/ti/omx/ducati/domx/system/mm_osal/inc \
    external/libxml2/include \
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

LOCAL_CFLAGS += -fno-short-enums -DTARGET_OMAP4

LOCAL_MODULE:= libomxcameraadapter
LOCAL_MODULE_TAGS:= optional

include $(BUILD_SHARED_LIBRARY)

#######

#
# OMX Camera HAL 
#

#LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    CameraHal_Module.cpp

LOCAL_C_INCLUDES += \
    bionic/libc/include \
    frameworks/base/include/ui \
    frameworks/base/include/utils \
    hardware/ti/omap4xxx/camera/inc \
    hardware/ti/omap4xxx/libtiutils \
    hardware/ti/omap4xxx/hwc \
    hardware/ti/omx/ducati/domx/system/omx_core/inc \
    hardware/ti/omx/ducati/domx/system/mm_osal/inc \
    hardware/ti/tiler \
    hardware/ti/syslink/ipc-listener \
    external/libxml2/include \
    external/icu4c/common \


LOCAL_SHARED_LIBRARIES := \
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

LOCAL_STATIC_LIBRARIES := \
    libxml2 \

LOCAL_CFLAGS += -fno-short-enums -DCOPY_IMAGE_BUFFER -DTARGET_OMAP4 -mfpu=neon

LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
LOCAL_MODULE := camera.$(TARGET_BOARD_PLATFORM)
LOCAL_MODULE_TAGS := optional

include $(BUILD_HEAPTRACKED_SHARED_LIBRARY)

endif
