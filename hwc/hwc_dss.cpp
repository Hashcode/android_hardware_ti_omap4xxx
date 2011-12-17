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
#include <assert.h>

#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>
#include <linux/fb.h>

#include <hardware/hardware.h>
#include <hardware/hwcomposer.h>
#include <cutils/native_handle.h>

#include "hwc_dss.h"
#include "hwc_priv.h"
#include "hwc_buffers.h"
#include "ti_hwc_ioctl.h"
#include "hwc_hdmi.h"

#define MAX_OVERLAYS 4

/*
 * this is to mark temporary code for hdmi cloning
 */
// #define HDMI_DEMO

/*
 * struct pipeline_state::pipe
 */
enum {
    HWCIMPL_PIPE_GFX = 0,
    HWCIMPL_PIPE_VID1 = 1,
    HWCIMPL_PIPE_VID2 = 2,
    HWCIMPL_PIPE_VID3 = 3,
    HWCIMPL_PIPE_MAX,  /* Number of pipelines */
};

/*
 * struct pipeline_state::flags
 */
enum {
    /* pipeline is available */
    HWCIMPL_PIPE_UNUSED = 0x0,
    /* pipeline is not available for use by the HWC */
    HWCIMPL_PIPE_UNAVAILABLE = 0x1,
    /* pipeline is being used by the HWC */
    HWCIMPL_PIPE_INUSE = 0x2,
    /* pipeline marked by prepare in HWC */
    HWCIMPL_PIPE_PREPARED = 0x4,
};

struct pipeline_state {
    hwc_layer *layer;
    int flags;
    int layer_no;
    int z;
};

struct display_info {
    int width;
    int height;
};

/* Create a 'right sized' data structure for the dss_hwc_set_info interface */
struct dss_hwc_set_info_container {
    struct dss_hwc_set_info s;
    struct dss_hwc_ovl_info ovls[MAX_OVERLAYS];
};

/* Global pipeline state */
static struct pipeline_state g_pipelines[HWCIMPL_PIPE_MAX];
static int g_top_vid_pipe = HWCIMPL_PIPE_VID1;

/* Information about displays */
static struct display_info g_displays[HWC_PLAT_DISPLAY_NO];

/* Ioctl device */
static int g_devfd;

/* Misc debug stuff */
static int calls_to_prepare_before_set = 0;

/* Force rgb layers to be rendered to the framebuffer */
static int g_force_rgb_to_fb = 1;

static void dump_layer(int i, hwc_layer_t const* l) {
    IMG_native_handle_t *handle = (IMG_native_handle_t *) l->handle;

    HWC_UNUSEDV(handle);
    HWC_UNUSEDV(i);

    LOGV_IF(handle != NULL,
        "\tLayer= %d, type=%d, flags=%08x, handle=%p, tr=%02x, blend=%04x, "
        "usage=%08x, bpp=%i format=%d vptr=%p pid=%d m_size=%d "
        "m_offest=%d queued=%d width=%d height=%d",
        i, l->compositionType, l->flags, handle, l->transform, l->blending,
        handle->usage, handle->uiBpp, handle->iFormat, handle->vptr,handle->pid,
        handle->m_size, (int)handle->m_offset, handle->queued, handle->iWidth,
        handle->iHeight);
}


/* Here is where dss structure will be updated with buffer info */
static int update_buffer(int pipe_idx, hwc_cached_buffer_t *cbuf, struct dss_hwc_ovl_info* pipe_dss_info)
{
    hwc_layer_t *layer = g_pipelines[pipe_idx].layer;
    IMG_native_handle_t* layer_buffer = (IMG_native_handle_t*) layer->handle;

    /* convert transformation */
    pipe_dss_info->mirror = !!(layer->transform & HWC_TRANSFORM_FLIP_H);

    if(cbuf->type == BUF_USER) {
        /* mirroring is only supported for tiler 2D buffers */
        pipe_dss_info->mirror = 0;
    }

    pipe_dss_info->rotation = (layer->transform & HWC_TRANSFORM_FLIP_V) ? 2 : 0;
    if (pipe_dss_info->rotation)
        pipe_dss_info->mirror = !pipe_dss_info->mirror;

    if (layer->transform & HWC_TRANSFORM_ROT_90) {
        if (pipe_dss_info->mirror)
            pipe_dss_info->rotation = (pipe_dss_info->rotation - 1) & 3;
        else
            pipe_dss_info->rotation++;
    }

    /* convert color format */
    switch (layer_buffer->iFormat) {
        case HAL_PIXEL_FORMAT_RGBX_8888:
            pipe_dss_info->color_mode = OMAP_DSS_COLOR_RGB24U;
            break;
        case HAL_PIXEL_FORMAT_RGBA_8888:
        case HAL_PIXEL_FORMAT_BGRA_8888:
            pipe_dss_info->color_mode = (layer->blending == HWC_BLENDING_NONE) ? OMAP_DSS_COLOR_RGB24U : OMAP_DSS_COLOR_ARGB32;
            break;
        case HAL_PIXEL_FORMAT_RGB_888:
            pipe_dss_info->color_mode = OMAP_DSS_COLOR_RGB24P;
            break;
        case HAL_PIXEL_FORMAT_RGB_565:
            pipe_dss_info->color_mode = OMAP_DSS_COLOR_RGB16;
            break;
        case HAL_PIXEL_FORMAT_RGBA_5551:
            pipe_dss_info->color_mode = (layer->blending == HWC_BLENDING_NONE) ? OMAP_DSS_COLOR_XRGB15 : OMAP_DSS_COLOR_ARGB16_1555;
            break;
        case HAL_PIXEL_FORMAT_RGBA_4444:
            pipe_dss_info->color_mode = (layer->blending == HWC_BLENDING_NONE) ? OMAP_DSS_COLOR_RGB12U : OMAP_DSS_COLOR_ARGB16;
            break;
        case HAL_PIXEL_FORMAT_NV12:
            pipe_dss_info->color_mode = OMAP_DSS_COLOR_NV12;
            break;
        case HAL_PIXEL_FORMAT_YCbCr_422_SP:
            pipe_dss_info->color_mode = OMAP_DSS_COLOR_YUV2;
            break;
        case HAL_PIXEL_FORMAT_YCbCr_422_I:
            pipe_dss_info->color_mode = OMAP_DSS_COLOR_UYVY;
            break;
        default:
            return -EINVAL;
    }

    pipe_dss_info->pre_mult_alpha = layer->blending == HWC_BLENDING_PREMULT;

    /* display position */
    pipe_dss_info->pos_x = layer->displayFrame.left;
    pipe_dss_info->pos_y = layer->displayFrame.top;
    pipe_dss_info->out_width = layer->displayFrame.right - layer->displayFrame.left;
    pipe_dss_info->out_height = layer->displayFrame.bottom - layer->displayFrame.top;

    /* crop */
    pipe_dss_info->crop_x = layer->sourceCrop.left;
    pipe_dss_info->crop_y = layer->sourceCrop.top;
    pipe_dss_info->crop_w = layer->sourceCrop.right - layer->sourceCrop.left;
    pipe_dss_info->crop_h = layer->sourceCrop.bottom - layer->sourceCrop.top;

    pipe_dss_info->enabled = 1;
    pipe_dss_info->global_alpha = 255;

    /* for now interlacing and vc1 info is not supplied */
    pipe_dss_info->ilace = OMAP_DSS_ILACE_NONE;
    pipe_dss_info->vc1.enable = 0;
    pipe_dss_info->zorder = g_pipelines[pipe_idx].z;
    pipe_dss_info->ix = pipe_idx;

    /* from buffer handle: */
    pipe_dss_info->width = layer_buffer->iWidth;
    pipe_dss_info->height = layer_buffer->iHeight;
    pipe_dss_info->handle = (__u32) cbuf->mapped_ptr;

    if(cbuf->type == BUF_USER)
        pipe_dss_info->stride = layer_buffer->iWidth * layer_buffer->uiBpp;
    else /*BUF_TILER*/
        pipe_dss_info->stride = 4096;

    /* for NV12 buffers we need to specify location of UV buffer */
    if(pipe_dss_info->color_mode == OMAP_DSS_COLOR_NV12)
        pipe_dss_info->module = pipe_dss_info->handle + (pipe_dss_info->stride * pipe_dss_info->height);

    LOGV("ovl%d(%s z%d %08x%s *%d%% %d*%d:%d,%d+%d,%d rot%d%s => %d,%d+%d,%d %x/%x|%d)\n",
                   pipe_dss_info->ix, pipe_dss_info->enabled ? "ON" : "off", pipe_dss_info->zorder,
                   pipe_dss_info->color_mode, pipe_dss_info->pre_mult_alpha ? " premult" : "",
                   (pipe_dss_info->global_alpha * 100 + 128) / 255,
                   pipe_dss_info->width, pipe_dss_info->height, pipe_dss_info->crop_x, pipe_dss_info->crop_y, pipe_dss_info->crop_w, pipe_dss_info->crop_h,
                   pipe_dss_info->rotation, pipe_dss_info->mirror ? "+mir" : "",
                   pipe_dss_info->pos_x, pipe_dss_info->pos_y, pipe_dss_info->out_width, pipe_dss_info->out_height,
                   pipe_dss_info->handle, pipe_dss_info->module, pipe_dss_info->stride);
    return 0;
}

/*******************************************************************
 * start implementation of external functions exposed by hwc hal api
 *******************************************************************/

/*
 * This function will be called by HWC prepare, it will understand
 * the currently available video pipelines and mark those layers
 * it intends to issue to an overlay
 */
int prepare_dss_layers(hwc_layer_list_t *list)
{
//    LOGV("->prep"); /* Noisy logging */
    int n, nlayers;
    int next_pipe = g_top_vid_pipe;

    calls_to_prepare_before_set++;

    if (list == NULL) {
        return 0;
    }

    nlayers = list->numHwLayers;

    if (list->flags & HWC_GEOMETRY_CHANGED) {
        LOGV("PREPARE: Geometry changed, parsing %i ", nlayers);
        /*
         * TODO: If the geometry changed we need to unmap buffers
         * from the layers we used with HWC_OVERLAY previously, a more
         * sophisticated method would be to have a cache of buffers that have
         * been mapped before to prevent remapping on every geometry change
         */
        unmap_cached_buffers();
    }

#ifdef HDMI_DEMO
    /*
     * If external display connected (hdmi) we reserve VID1/OVL1 to UI FB cloning.
     * In that case video is passed through VID2/OVL2.
     * If no external display - video is passed through VID1/OVL1.
     */
    if (list->flags & HWC_EXTERNAL_DISPLAY_CONNECTED) {
        g_top_vid_pipe = HWCIMPL_PIPE_VID2;
        /*
         * Still need this call here for UI cloning to start working immediately,
         * since "set" function will not be executed until UI changed.
         */
        hwc_hdmi_clone(true);
    }
    else{
        g_top_vid_pipe = HWCIMPL_PIPE_VID1;
    }
#endif

    int state_ignore_rgb = g_force_rgb_to_fb;

parse_layers_again:
    int startz = 1;

    for (n=0; n<nlayers; n++) {
//      LOGV("%i", n); /* Noisy logging */
        hwc_layer_t *layer = &list->hwLayers[n];
        IMG_native_handle_t *handle = (IMG_native_handle_t *) layer->handle;

        if (handle == NULL) {
            LOGV("Null layer: %d", n);
            continue;
        }

        if (layer->flags & HWC_SKIP_LAYER) {
            LOGV("Skipping layer: %d", n);
            continue;
        }

        /*
         * Inspect the visible region - 2nd guessing from old
         * surfaceflinger * behaviour that we might need to ignore
         * layers which are not visible
         */
        hwc_region_t *region = &layer->visibleRegionScreen;

        if (!region->numRects || !region->rects) {
            LOGV("DSS skipping layer: %d", n);
            continue;
        }

        /*
         * Assume that layers with multiple rectangle regions of a single
         * buffer can't be handled by the DSS
         */
        if (region->numRects > 1) {
            LOGV("DSS skip multi rect layer: %d, %d", n, region->numRects);
            continue;
        }

        if (handle->iFormat != HAL_PIXEL_FORMAT_NV12) {
             if (state_ignore_rgb)
                 continue;
             /* Valid RGB formats  - this will be expanded later. For now
                only allow RGB565 */
             if (handle->iFormat != HAL_PIXEL_FORMAT_RGB_565)
                 continue;
        }

        if (!state_ignore_rgb && (next_pipe >= HWCIMPL_PIPE_MAX)) {
            state_ignore_rgb = 1;
            goto parse_layers_again;
        }

        if (next_pipe >= HWCIMPL_PIPE_MAX)
            break;

        g_pipelines[next_pipe].layer = layer;
        g_pipelines[next_pipe].flags = HWCIMPL_PIPE_INUSE;
        g_pipelines[next_pipe].layer_no = n;
        g_pipelines[next_pipe].z = startz++;
        next_pipe++;

    } /* End parse layer state */

    /* Reset state for the unused pipes */
    int extra_pipes = next_pipe;
    while(extra_pipes < HWCIMPL_PIPE_MAX) {
        g_pipelines[extra_pipes].layer = NULL;
        g_pipelines[extra_pipes].flags = HWCIMPL_PIPE_UNUSED;
        extra_pipes++;
    }

    int pipes_used = next_pipe - g_top_vid_pipe;
    int last_pipe = next_pipe - 1;
    int clear_fb = pipes_used <= nlayers ? 0 : 1;

    /*
     * Hack gets the FB cleared for us so GFX pipeline is transparent - this
     * is in place to see the true performance of 3D rendering without SF
     * clearing the frame buffer

    static int firstfewframes=40;
    if(firstfewframes-- > 0)
        clear_fb = 1;
     */

    /*
     * For now, require that the FB is cleared and get surfaceflinger to clear
     * the framebuffer with alpha.
     *
     * We have further framebuffer related issues:
     *
     * the first problem we want to address is if there is no framebuffer
     * content we might want to disable the GFX pipeline. Obviously this
     * needs to be active when we do need to render to the FB.
     *
     * Secondly, though slightly separate we may have layer content which is
     * not new and we wish to avoid redrawing, in this case we might pretend
     * that a layer is going to the OVERLAY as far as surfaceflinger is
     * concerned but actually do nothing.
     *
     * Thirdly, also consider the case where just overlays are being used
     * and you have no work for it, but do have work for the framebuffer.
     */
    clear_fb = 1; /* XXX */

    LOGV("last_pipe %d pipes_used %d clear_fb %d",
         last_pipe, pipes_used, clear_fb);

    if (next_pipe == g_top_vid_pipe) {
        goto end_fb; /* No pipelines used */
    }

    for (next_pipe = g_top_vid_pipe; next_pipe <= last_pipe; next_pipe++) {
        hwc_cached_buffer_t cbuf;
        hwc_layer_t *layer = g_pipelines[next_pipe].layer;
        if (!layer)
            break;

        if(get_cached_buffer(layer, &cbuf) < 0) {
            if (map_cached_buffer(layer) != 0) {

                LOGE("map cached failed");
                if (!state_ignore_rgb) {
                    /*
                     * The only normal reason to hit this condition is that
                     * we have run out of Tiler-1D space. This is bad because
                     * it should be properly budgeted for so that both the HAL
                     * and any other users of tiler do not exhaust this memory
                     * pool.
                     */
                    state_ignore_rgb=1;
                    goto parse_layers_again;
                }
            }
        }
        LOGV("Going to ovl");
        layer->compositionType = HWC_OVERLAY;
        if(clear_fb)
            layer->hints |= HWC_HINT_CLEAR_FB;
    }

end_fb:
    if (g_pipelines[HWCIMPL_PIPE_GFX].flags & HWCIMPL_PIPE_UNUSED) {
        if (clear_fb)
            g_pipelines[HWCIMPL_PIPE_GFX].flags |= HWCIMPL_PIPE_INUSE;
    }

    return 0;
}

/*
 * This function will be called by HWC set, it handles those layers marked
 * in the prepare operation and present those layers with the DSS
 */
int set_dss_layers(hwc_layer_list_t *list)
{
//    LOGV("->set"); /* Noisy logging */

    int nlayers;
    int n_ovls = 0;
    int ret = 0;
    struct dss_hwc_set_info *s = NULL;

    calls_to_prepare_before_set = 0;

    if (list == NULL)
        return 0;

#ifdef HDMI_DEMO
    // first of all verify if external display connected
    if (list->flags & HWC_EXTERNAL_DISPLAY_CONNECTED) {
        hwc_hdmi_clone(true);
    }
    else{
        hwc_hdmi_clone(false);
    }
#endif

    nlayers = list->numHwLayers;

    dss_hwc_set_info_container setinfo;
    bzero(&setinfo, sizeof(setinfo));
    s = &setinfo.s;

    s->mgr.alpha_blending = 1;
    int next_pipe = g_top_vid_pipe;

    /* All these are OVERLAY layers */
    while (g_pipelines[next_pipe].layer) {

        hwc_cached_buffer_t cbuf;
        hwc_layer_t *layer = g_pipelines[next_pipe].layer;
        dump_layer(g_pipelines[next_pipe].layer_no, layer);

        if (get_cached_buffer(layer, &cbuf) < 0) {
            IMG_native_handle_t *hndl = (IMG_native_handle_t*)layer->handle;
            LOGE("Buffer %p for this layer is not mapped!", hndl->vptr);
            /* No failure mode for this, SurfaceFlinger has started off
               the rendering of the FB layers already */
        } else {
            int err = update_buffer(next_pipe, &cbuf, s->ovls+n_ovls);
            if (err)
                LOGE("Error presenting buffer %p", cbuf.layer_buffer);
            else{
                LOGV("Updated buffer %p", cbuf.layer_buffer);
                n_ovls++;
            }
        }
        next_pipe++;
    }

    if(n_ovls > 0) {
        s->num_ovls = n_ovls;
        s->update = true;

        LOGV("mgr(dis%d alpha=%d col=%08x ilace=%d)\n", s->mgr.ix, s->mgr.alpha_blending, s->mgr.default_color, s->mgr.interlaced);
        LOGV("set(udpate%d x=%d y=%d w=%d h=%d num_ovls=%d)\n", s->update, s->x, s->y, s->w, s->h, s->num_ovls);

        ret = ioctl(g_devfd, OMAPDSS_HWC_SET, s);
        if(ret)
            LOGE("Error OMAPDSS_HWC_SET %d", ret);

#ifdef HDMI_DEMO
        if (list->flags & HWC_EXTERNAL_DISPLAY_CONNECTED) {
            struct dss_hwc_set_info *s1 = NULL;
            dss_hwc_set_info_container setinfo1;
            memcpy(&setinfo1, &setinfo, sizeof(setinfo));
            s1 = &setinfo1.s;

            s1->mgr.ix = 2;
            s1->ovls[0].ix = 3;
            s1->ovls->pos_x= 0;
            s1->ovls->pos_y= 0;
            s1->ovls[0].out_width = 1920;
            s1->ovls[0].out_height = 1080;
            s1->ovls[0].enabled = 1;
            ret = ioctl(g_devfd, OMAPDSS_HWC_SET, s1);
            if(ret)
                LOGE("Error OMAPDSS_HWC_SET on tv %d", ret);
        }
    }

    else {
        if (list->flags & HWC_EXTERNAL_DISPLAY_CONNECTED) {
            disable_ext_video();
        }
#endif
    }


    return ret;
}

/*
 * Called when the HWC is opened
 */
int init_hwc_dss()
{
    int rv = 0;
    struct fb_var_screeninfo fb_var;
    struct fb_fix_screeninfo fb_fix;

    /* Open framebuffer here to talk directly to the DSS */
    g_devfd = open(FB_DEV_NAME, O_RDWR);
    if (g_devfd < 0)
        g_devfd = open(FB_DEV_NAME_FALLBACK, O_RDWR);

    if (g_devfd < 0) {
        LOGE("unable to open framebuffer %s %s", FB_DEV_NAME,
            FB_DEV_NAME_FALLBACK);
        rv = -EINVAL;
        goto end;
    }

    /* Get framebuffer dimensions */
    if (ioctl(g_devfd, FBIOGET_FSCREENINFO, &fb_fix)) {
        rv = -EINVAL;
        goto end;
    }

    if (ioctl(g_devfd, FBIOGET_VSCREENINFO, &fb_var)) {
        rv = -EINVAL;
        goto end;
    }

    /* Main display only */
    g_displays[0].width = fb_var.xres;
    g_displays[0].height = fb_var.yres;

    LOGD("Display size is x = %i, y = %i", g_displays[0].width, g_displays[0].height);

    init_cached_buffers();
end:
    if (rv != 0 && g_devfd)
        close(g_devfd);
    return rv;
}

/*
 * Called when the HWC is closed
 */
int deinit_hwc_dss()
{
    /* TODO: Wait here until all buffers have been released */
    if (g_devfd >= 0)
        close(g_devfd);
    return 0;
}
