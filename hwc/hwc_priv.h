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

#ifndef _HWC_PRIV_H
#define _HWC_PRIV_H

#include <hardware/hwcomposer.h>
#include <cutils/log.h>
#include "hal_public.h"

typedef struct ti_hwc_module {
    hwc_module_t base;
    /* TI hw module data goes here */
} ti_hwc_module_t;

typedef struct ti_hwc_composer_device {
    hwc_composer_device_t base;
    /* TI hwc module data goes here */
} ti_hwc_composer_device_t;

int ti_hwc_prepare(struct hwc_composer_device *dev,
                hwc_layer_list_t* list);

int ti_hwc_set(struct hwc_composer_device *dev, hwc_display_t dpy,
               hwc_surface_t sur, hwc_layer_list_t* list);

void check_showfps(void);

#ifndef HAL_PIXEL_FORMAT_NV12
#define HAL_PIXEL_FORMAT_NV12 0x100
#else
#if HAL_PIXEL_FORMAT_NV12 != 0x100
#error HAL_PIXEL_FORMAT_NV12 has an unexpected value
#endif /* format check */
#endif

#define HWC_UNUSED(expr) do { (void)(expr); } while (0)

/*
 * The purpose of HWC_UNUSEDV is to handle variables which
 * might be referenced only in verbosely logged code. The verbose logging
 * macros themselves may be empty when LOG_NDEBUG is non-zero
 */
#if LOG_NDEBUG
#define HWC_UNUSEDV(expr) HWC_UNUSED((expr))
#else
#define HWC_UNUSEDV(expr)
#endif
#endif // _HWC_PRIV_H_
