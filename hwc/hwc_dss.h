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


#include "hwc_priv.h"

#ifndef _HWC_DSS_H
#define _HWC_DSS_H

/*
 * Platform specific definitions for the HWC HAL implentation
 * on OMAP. This file should be overlayed with device specific
 * files in Android product builds.
 */

/*
 * Number of displays
 * - Not including external / dynamically attached displays
 */
#define HWC_PLAT_DISPLAY_NO 1

/*
 * Number of external displays
 * - Dynamically attached, may not be plugged in currently
 */
#define HWC_PLAT_EXTERNALDISPLAY_NO 1

/* Framebuffer device node names */
#define FB_DEV_NAME "/dev/graphics/fb0"
#define FB_DEV_NAME_FALLBACK "/dev/graphics/fb0"

int init_hwc_dss(void);
int deinit_hwc_dss(void);
int prepare_dss_layers(hwc_layer_list_t *list);
int set_dss_layers(hwc_layer_list_t *list);

#endif // _HWC_DSS_H
