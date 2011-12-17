/*
 * Copyright (C) Texas Instruments - http://www.ti.com/
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include <fcntl.h>
#include <errno.h>

#include <hardware/hardware.h>
//#include <hardware/overlay.h>
#include <hardware/hwcomposer.h>

#include "hwc_priv.h"
#include "hwc_dss.h"

static int hwc_device_open(const hw_module_t* module, const char* name,
                hw_device_t** device);

static struct hw_module_methods_t hwc_module_methods = {
        open: hwc_device_open
};

ti_hwc_module_t HAL_MODULE_INFO_SYM = {
    base: {
        common: {
            tag: HARDWARE_MODULE_TAG,
            version_major: 1,
            version_minor: 0,
            id: HWC_HARDWARE_MODULE_ID,
            name: "TI Hardware Composer HAL",
            author: "Texas Instruments",
            methods: &hwc_module_methods,
            dso: NULL,
            reserved: { 0 }
        },
    },
};

static int hwc_device_close(hw_device_t* device)
{
    ti_hwc_composer_device_t* hwc_device;
    LOGI("TI hwc_device close");
    hwc_device = (ti_hwc_composer_device_t*)device;

    if (hwc_device)
        free(hwc_device);

    return deinit_hwc_dss();
}

int hwc_device_open(const hw_module_t* module, const char* name,
                hw_device_t** device)
{
    int rv = -EINVAL;
    ti_hwc_composer_device_t *hwc_device;
    LOGI("hwc_device open");

    if (!strcmp(name, HWC_HARDWARE_COMPOSER)) {

        hwc_device = (ti_hwc_composer_device_t*)malloc(sizeof(*hwc_device));
        if (!hwc_device) {
            return -ENOMEM;
        }
        memset(hwc_device, 0, sizeof(*hwc_device));

        hwc_device->base.common.tag = HARDWARE_DEVICE_TAG;
        hwc_device->base.common.version = 0;
        hwc_device->base.common.module = (hw_module_t *)(module);
        hwc_device->base.common.close = hwc_device_close;
        hwc_device->base.prepare = ti_hwc_prepare;
        hwc_device->base.set = ti_hwc_set;
        *device = &hwc_device->base.common;

        check_showfps();
        rv = init_hwc_dss();
    }

    return rv;
}

