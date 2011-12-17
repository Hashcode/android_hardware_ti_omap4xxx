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


#include <errno.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>

#include "hwc_hdmi.h"
#include "hwc_priv.h"
#include "ti_hwc_ioctl.h"



/*
 * Notes for Currently supported UI-Cloning-Feature
 *
 * 1. fb0 is on Overlay0 which is on LCD1.
 * 2. fb1 is on Overlay1 which can go to LCD2 or TV.
 * 3. UI-Cloning is supported on LCD2 and TV.
 * 4. Cloning of only fb0 is supported.
 * 5. For cloning of fb0 on Overlay1, fb1 is switched off.
 */

const char fb0Overlays [PATH_MAX] = "/sys/class/graphics/fb0/overlays";
const char fb1Overlays [PATH_MAX] = "/sys/class/graphics/fb1/overlays";
const char fb0FitToScreenOption[PATH_MAX] = "/sys/class/graphics/fb0/fit_to_screen";

const char overlay1Manager[PATH_MAX] = "/sys/devices/platform/omapdss/overlay1/manager";
const char overlay1Alpha[PATH_MAX]   = "/sys/devices/platform/omapdss/overlay1/global_alpha";
const char overlay1Enabled[PATH_MAX] = "/sys/devices/platform/omapdss/overlay1/enabled";
const char overlay1ZOrder[PATH_MAX]  = "/sys/devices/platform/omapdss/overlay1/zorder";

const char overlay3Enabled[PATH_MAX] = "/sys/devices/platform/omapdss/overlay3/enabled";

enum displayId {
    DISPLAYID_NONE = -1,
    DISPLAYID_LCDSECONDARY = 1,
    DISPLAYID_TVHDMI = 2,
};

enum OverlayZorder {
    OMAP_DSS_OVL_ZORDER_0 = 0x0,
    OMAP_DSS_OVL_ZORDER_1 = 0x1,
    OMAP_DSS_OVL_ZORDER_2 = 0x2,
    OMAP_DSS_OVL_ZORDER_3 = 0x3,
};


int sysfile_write(const char* pathname, const void* buf, size_t size) {
    int fd = open(pathname, O_WRONLY);
    if (fd == -1) {
        LOGE("Can't open [%s]", pathname);
        return -1;
    }
    size_t written_size = write(fd, buf, size);
    if (written_size < size) {
        LOGE("Can't write [%s]", pathname);
        close(fd);
        return -1;
    }
    if (close(fd) == -1) {
        LOGE("cant close [%s]", pathname);
        return -1;
    }
    return 0;
}

/**
* a method to Clone UI to the requested Display Id.
*
* Input: integer between 0 and 3
* 0 - PRIMARY DISPLAY
* 1 - SECONDARY DISPLAY
* 2 - HDTV
* 3 - pico DLP
* these IDs should be in sync with overlay enums
**/
static void clone_ui2HDMI(int displayId) {
    LOGD("UiCloningService_CloneUiToDisplay : DisplayId= [%d]", displayId);

    // Clone UI on Other Display
    if(displayId == DISPLAYID_TVHDMI) {
        if (sysfile_write(overlay1Enabled, "0", sizeof("0")) < 0) {
            LOGE("Failed to set overlay1/enabled = 0");
            goto end;
        }
        if (sysfile_write(overlay1Manager, "tv", sizeof("tv")) < 0) {
            LOGE("Failed to set overlay/manager = tv");
            goto end;
        }
        if (sysfile_write(fb1Overlays, "", sizeof("")) < 0) {
            LOGE("Failed to set fb1/overlays = NULL");
            goto end;
        }
        if (sysfile_write(fb0Overlays, "0,1", sizeof("0,2")) < 0) {
            LOGE("Failed to set fb0/overlays = 0,1");
            goto end;
        }
        if (sysfile_write(fb0FitToScreenOption, "1", sizeof("1")) < 0) {
            LOGE("Failed to set fb0/fit_to_screen = 1");
            goto end;
        }
        char overlay1ZOrderValue[5];
        sprintf(overlay1ZOrderValue, "%d", OMAP_DSS_OVL_ZORDER_0);
        if (sysfile_write(overlay1ZOrder, overlay1ZOrderValue, sizeof("1")) < 0) {
            LOGE("Failed to set overlay1/zorder = 0");
            goto end;
        }
        if (sysfile_write(overlay1Enabled, "1", sizeof("1")) < 0) {
            LOGE("Failed to set overlay1/enabled = 1");
            goto end;
        }
    }
    // Stop cloning UI on Other Display
    else if(displayId == DISPLAYID_NONE) {
        if (sysfile_write(overlay1Enabled, "0", sizeof("0")) < 0) {
            LOGE("Failed to set overlay1/enabled = 0");
            goto end;
        }
        if (sysfile_write(fb0Overlays, "0", sizeof("0")) < 0) {
            LOGE("Failed to set fb0/overlays = 0");
            goto end;
        }
        if (sysfile_write(fb0FitToScreenOption, "0", sizeof("0")) < 0) {
            LOGE("Failed to set fb0/fit_to_screen = 0");
            goto end;
        }
        if (sysfile_write(fb1Overlays, "1", sizeof("1")) < 0) {
            LOGE("Failed to set fb1/overlays = 1");
            goto end;
        }
        if (sysfile_write(overlay1Manager, "2lcd", sizeof("2lcd")) < 0) {
            LOGE("Failed to restore overlay/manager = 2lcd");
            goto end;
        }
    }

end:
    return;
}




/*******************************************************************
 * start implementation of external functions exposed by hwc_hdmi
 *******************************************************************/

/*
 * Called when the HWC is opened
 */
int hwc_hdmi_clone(bool isStart)
{
    static bool prev = false;
    if(prev != isStart)
    {
        prev = isStart;
        clone_ui2HDMI(isStart?DISPLAYID_TVHDMI:DISPLAYID_NONE);
    }
    return 0;
}

void disable_ext_video()
{
    if (sysfile_write(overlay3Enabled, "0", sizeof("0")) < 0) {
        LOGE("Failed to set overlay1/enabled = 0");
    }
}
