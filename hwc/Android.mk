# Copyright (C) 2011 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


LOCAL_PATH := $(call my-dir)

# HAL module implementation, not prelinked and stored in
# hw/<HWCOMPOSE_HARDWARE_MODULE_ID>.<ro.product.board>.so
include $(CLEAR_VARS)
LOCAL_PRELINK_MODULE := false
LOCAL_ARM_MODE := arm
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
LOCAL_SHARED_LIBRARIES := liblog libEGL libcutils libtimemmgr libutils
LOCAL_SRC_FILES := hwc_init.cpp hwc.cpp hwc_dss.cpp hwc_buffers.cpp hwc_hdmi.cpp

LOCAL_MODULE_TAGS := optional
LOCAL_C_INCLUDES := \
    hardware/ti/omap4xxx/tiler \
    hardware/ti/omap4xxx/include

LOCAL_MODULE := hwcomposer.omap4
LOCAL_CFLAGS := -DLOG_TAG=\"ti_hwc\" -Wall -Werror
# LOG_NDEBUG=0 means verbose logging enabled
LOCAL_CFLAGS += -DLOG_NDEBUG=0
include $(BUILD_SHARED_LIBRARY)
