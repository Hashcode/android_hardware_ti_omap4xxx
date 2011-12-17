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
#include <malloc.h>
#include <stdlib.h>

#include <cutils/properties.h>
#include <hardware/hardware.h>
//#include <hardware/overlay.h>
#include <hardware/hwcomposer.h>
#include <EGL/egl.h>
#include <utils/Timers.h>

#include "hwc_priv.h"
#include "hwc_dss.h"

#define UNLIKELY( x ) (__builtin_expect( (x), 0 ))

static int g_showfps;

void check_showfps(void)
{
    char value[PROPERTY_VALUE_MAX];
    property_get("debug.hwc.showfps", value, "0");
    g_showfps = atoi(value);
    LOGI_IF(g_showfps, "showfps = %d", g_showfps);
}

static void showfps(void)
{
    static int framecount = 0;
    static int lastframecount = 0;
    static nsecs_t lastfpstime = 0;
    static float fps = 0;
    framecount++;
    if (!(framecount & 0x1F)) {
        nsecs_t now = systemTime(SYSTEM_TIME_MONOTONIC);
        nsecs_t diff = now - lastfpstime;
        fps = ((framecount - lastframecount) * (float)(s2ns(1))) / diff;
        lastfpstime = now;
        lastframecount = framecount;
        LOGI("%d Frames, %f FPS", framecount, fps);
    }
}

static void do_showfps()
{
    if (UNLIKELY(g_showfps))
        showfps();
}

int ti_hwc_prepare(struct hwc_composer_device *dev, hwc_layer_list_t* list)
{
    int ret;
    HWC_UNUSED(dev);

    ret = prepare_dss_layers(list);

    return ret;
}

int ti_hwc_set(struct hwc_composer_device *dev, hwc_display_t dpy,
                                    hwc_surface_t sur, hwc_layer_list_t* list)
{
    HWC_UNUSED(dev);
    /*
     * TODO: default implementation, here we need to check if all the visible
     * layers are going to be handled by the DSS, if that's the case we don't
     * need to call eglSwapBuffers. If at least one layer is marked as
     * HWC_FRAMEBUFFER we need to call eglSwapBuffers.
     */

    set_dss_layers(list);

    EGLBoolean sucess = eglSwapBuffers((EGLDisplay)dpy, (EGLSurface)sur);
    if (!sucess)
        return HWC_EGL_ERROR;

    do_showfps();

    return 0;
}
