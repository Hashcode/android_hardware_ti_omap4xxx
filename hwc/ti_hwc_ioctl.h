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


#include <linux/ioctl.h>
#include <linux/types.h>

/* enumerations from display.h */
enum omap_color_mode {
	OMAP_DSS_COLOR_CLUT1		= 1 << 0,  /* BITMAP 1 */
	OMAP_DSS_COLOR_CLUT2		= 1 << 1,  /* BITMAP 2 */
	OMAP_DSS_COLOR_CLUT4		= 1 << 2,  /* BITMAP 4 */
	OMAP_DSS_COLOR_CLUT8		= 1 << 3,  /* BITMAP 8 */
	OMAP_DSS_COLOR_RGB12U		= 1 << 4,  /* xRGB12-4444 */
	OMAP_DSS_COLOR_ARGB16		= 1 << 5,  /* ARGB16-4444 */
	OMAP_DSS_COLOR_RGB16		= 1 << 6,  /* RGB16-565 */
	OMAP_DSS_COLOR_RGB24U		= 1 << 7,  /* xRGB24-8888 */
	OMAP_DSS_COLOR_RGB24P		= 1 << 8,  /* RGB24-888 */
	OMAP_DSS_COLOR_YUV2		= 1 << 9,  /* YUV2 4:2:2 co-sited */
	OMAP_DSS_COLOR_UYVY		= 1 << 10, /* UYVY 4:2:2 co-sited */
	OMAP_DSS_COLOR_ARGB32		= 1 << 11, /* ARGB32-8888 */
	OMAP_DSS_COLOR_RGBA32		= 1 << 12, /* RGBA32-8888 */
	OMAP_DSS_COLOR_RGBX24		= 1 << 13, /* RGBx24-8888 */
	OMAP_DSS_COLOR_NV12		= 1 << 14, /* NV12 format: YUV 4:2:0 */
	OMAP_DSS_COLOR_RGBA16		= 1 << 15, /* RGBA16-4444 */
	OMAP_DSS_COLOR_RGBX12		= 1 << 16, /* RGBx12-4444 */
	OMAP_DSS_COLOR_ARGB16_1555	= 1 << 17, /* ARGB16-1555 */
	OMAP_DSS_COLOR_XRGB15		= 1 << 18, /* xRGB15-1555 */
};

enum omap_dss_trans_key_type {
	OMAP_DSS_COLOR_KEY_GFX_DST = 0,
	OMAP_DSS_COLOR_KEY_VID_SRC = 1,
};

enum omap_dss_ilace_mode {
	OMAP_DSS_ILACE		= (1 << 0),	/* interlaced vs. progressive */
	OMAP_DSS_ILACE_SEQ	= (1 << 1),	/* sequential vs interleaved */
	OMAP_DSS_ILACE_SWAP	= (1 << 2),	/* swap fields, e.g. TB=>BT */

	OMAP_DSS_ILACE_NONE	= 0,
	OMAP_DSS_ILACE_IL_TB	= OMAP_DSS_ILACE,
	OMAP_DSS_ILACE_IL_BT	= OMAP_DSS_ILACE | OMAP_DSS_ILACE_SWAP,
	OMAP_DSS_ILACE_SEQ_TB	= OMAP_DSS_ILACE_IL_TB | OMAP_DSS_ILACE_SEQ,
	OMAP_DSS_ILACE_SEQ_BT	=  OMAP_DSS_ILACE_IL_BT | OMAP_DSS_ILACE_SEQ,
};

/* hwc ioctl structures shared with kernel */
struct dss_color_conv_info {
	int r_y, r_cr, r_cb;
	int g_y, g_cr, g_cb;
	int b_y, b_cr, b_cb;

	/* Y is 16..235, UV is 16..240 if not fullrange.  Otherwise 0..255 */
	__s32 fullrange;
};

struct dss_vc1_range_map_info {
	__s32 enable;	/* map or not? */

	__u32 range_y;	/* 0..7 */
	__u32 range_uv;	/* 0..7 */
};

struct dss_hwc_ovl_info {
	/* THIS PORTION OF THE CONFIGURATION MUST BE FILLED OUT TO CHECK DSS SUPPORT */

	__u32 ix;	/* ovl index same as sysfs/overlay# */

	/* :TBD: some way of specifying buffer address */
	__u32 handle;	/* for buffer_handle_t */
	__u32 module;	/* for gralloc_module_t */

	/* the following may be able to be derived from the handle or
	   may have to be specified */
	__u16 width;	/* buffer width */
	__u16 height;	/* buffer height */
	__u16 stride;	/* buffer stride */

	enum omap_color_mode color_mode;
	__s32 pre_mult_alpha;
	__u32 global_alpha;

	int rotation;	/* 0..3 (*90 degrees clockwise) */
	__s32 mirror;	/* left-to-right */
	enum omap_dss_ilace_mode ilace;	/* interlace mode */

	__s16 pos_x;
	__s16 pos_y;
	__u16 out_width;
	__u16 out_height;

	__u16 crop_x;
	__u16 crop_y;
	__u16 crop_w;
	__u16 crop_h;

	/* END OF PORTION OF THE CONFIGURATION TO CHECK DSS SUPPORT */


	/* THIS PORTION OF THE CONFIGURATION IS ONLY USED FOR SET */

	__u16 zorder;	/* 0..3 */
	__s32 enabled;

	struct dss_color_conv_info cconv;
	struct dss_vc1_range_map_info vc1;

	/* END OF PORTION OF THE CONFIGURATION USED FOR SET */
};

/*
 * Assumptions:
 *
 * 1) 0 <= crop_x <= crop_x + crop_w <= width
 * 2) 0 <= crop_y <= crop_y + crop_h <= height
 * 3) color_mode is supported by overlay
 * 4) requested scaling is supported by overlay and functional clocks
 *
 * Notes:
 *
 * 1) Any portions of X:[pos_x, pos_x + out_width] and
 *    Y:[pos_y, pos_y + out_height] outside of the screen
 *    X:[0, screen.width], Y:[0, screen.height] will be cropped
 *    automatically without changing the scaling ratio.
 *
 * 2) Crop region will be adjusted to the pixel granularity:
 *    (2-by-1) for YUV422, (2-by-2) for YUV420.  This will
 *    not modify the output region.  Crop region is for the
 *    original (unrotated) buffer, so it does not change with
 *    rotation.
 *
 * 3) Rotation will not modify the output region, specifically
 *    its height and width.  Also the coordinate system of the
 *    display is always (0,0) = top left.
 *
 * 4) cconv and vc1 only needs to be filled for YUV color modes.
 *
 * 5) vc1.range_y and vc1.range_uv only needs to be filled if
 *    vc1.enable is true.
 *
 */

struct dss_hwc_mgr_info {
	__u32 ix;	/* display index same as sysfs/display# */

	__s32 interlaced;

	__u32 default_color;

	__s32 alpha_blending;	/* overrides transparency */

	__s32 trans_enabled;
	enum omap_dss_trans_key_type trans_key_type;
	__u32 trans_key;
};

/*
 * Notes:
 *
 * 1) trans_key_type and trans_enabled only need to be filled if
 *    trans_enabled is true, and alpha_blending is false.
 */


struct dss_hwc_set_info {
	__s32 update;	/* calls display update after setting up compositor */
	__u16 x, y;	/* update region */
	__u16 w, h;	/* (set w/h to 0/0 for full screen) */
	__u32 num_ovls;	/* number of overlays used in the composition */
	struct dss_hwc_mgr_info mgr;
	struct dss_hwc_ovl_info ovls[0]; /* up to 5 overlays to set up */
};

/*
 * Notes:
 *
 * 1) x, y, w, h only needs to be set if update is true.
 */


/*
 * ioctl: OMAPDSS_HWC_SET, struct dss_compositor_info
 *
 * 1. sets manager of each ovl in composition to the display
 * 2. calls set_dss_ovl_info() for each ovl to set up the
 *    overlay staging structures (this is a wrapper around ovl->set_info())
 * 3. calls set_dss_mgr_info() for mgr to set up the manager
 *    staging structures (this is a wrapper around mgr->set_info())
 * 4. if update is true:
 *      calls manager->apply()
 *      calls driver->update() in a non-blocking fashion
 *      this will program the DSS synchronously
 */
#define OMAPDSS_HWC_SET  _IOW('O', 128, struct dss_hwc_set_info)

/**
 * Check if DSS supports this overlay configuration.
 *
 * Initial static inline implementation performs just color
 * format support and scaling support checks.  Note: for now
 * you can only rotate/mirror a TILER2D buffer.
 *
 * @param oi	overlay configuration to be checked.
 *
 * @return true if DSS supports this configuration
 */
static inline bool dss_hwc_ovl_supports(struct dss_hwc_ovl_info *oi) {
	return oi &&
		(oi->ix == 0 ?
		 /* GFX pipeline requirements */
		 (!(oi->color_mode & (OMAP_DSS_COLOR_YUV2 |
				      OMAP_DSS_COLOR_UYVY |
				      OMAP_DSS_COLOR_NV12)) &&
		  ((oi->rotation & 1) ?
		   (oi->crop_w == oi->out_height &&
		    oi->crop_h == oi->out_width) :
		   (oi->crop_w == oi->out_width &&
		    oi->crop_h == oi->out_height))) :
		 /* VID pipeline requirements */
		 (!(oi->color_mode & (OMAP_DSS_COLOR_CLUT1 |
				      OMAP_DSS_COLOR_CLUT2 |
				      OMAP_DSS_COLOR_CLUT4 |
				      OMAP_DSS_COLOR_CLUT8))));
}
