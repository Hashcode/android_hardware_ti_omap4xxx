# only include if running on an omap4 platform
ifeq ($(TARGET_BOARD_PLATFORM),omap4)

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false
LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES := \
		memmgr.c \
		tilermgr.c \


LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/ \

#LOCAL_CFLAGS += -pipe -fomit-frame-pointer -Wall  -Wno-trigraphs -Werror-implicit-function-declaration  -fno-strict-aliasing -mapcs -mno-sched-prolog -mabi=aapcs-linux -mno-thumb-interwork -msoft-float -Uarm -DMODULE -D__LINUX_ARM_ARCH__=7  -fno-common -DLINUX -fpic

LOCAL_MODULE    := libtimemmgr
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_PRELINK_MODULE := false
LOCAL_ARM_MODE := arm
LOCAL_SRC_FILES := memmgr_test.c testlib.c
LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/ \

LOCAL_SHARED_LIBRARIES := libtimemmgr
LOCAL_MODULE := memmgr_test
LOCAL_MODULE_TAGS := optional tests
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_PRELINK_MODULE := false
LOCAL_ARM_MODE := arm
LOCAL_SRC_FILES := utils_test.c testlib.c
LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/ \

LOCAL_SHARED_LIBRARIES := libtimemmgr
LOCAL_MODULE    := utils_test
LOCAL_MODULE_TAGS := optional tests
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_PRELINK_MODULE := false
LOCAL_ARM_MODE := arm
LOCAL_SRC_FILES := tiler_ptest.c
LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/ \

LOCAL_SHARED_LIBRARIES := libtimemmgr
LOCAL_MODULE    := tiler_ptest
LOCAL_MODULE_TAGS := optional tests
include $(BUILD_EXECUTABLE)

endif
