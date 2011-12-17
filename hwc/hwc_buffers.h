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
#ifndef _HWC_BUFFERS_H
#define _HWC_BUFFERS_H

/*
 * This module is responsible for managing buffers associated with a HWC
 * layer.
 *
 * Tiler-2D buffers are already mapped into the Tiler address space so
 * don't really require any special handling.
 *
 * However, other buffers such as RGB buffers will be dynamically mapped
 * into a limited region of Tiler-1D memory (effectively a cache). This is
 * done so they can be rendered directly by the DSS.
 */

typedef enum {
    BUF_TILER = 0,
    BUF_USER
} HWC_BUF_TYPE;

typedef struct hwc_cached_buffer hwc_cached_buffer_t;

typedef struct hwc_cache_list {
    hwc_cached_buffer_t *next;
//    hwc_cached_buffer_t *prev;
} hwc_cache_list_t;

typedef struct hwc_cached_buffer {
    hwc_cache_list_t l; /* private */
    IMG_native_handle_t* layer_buffer;
    void *mapped_ptr;
    HWC_BUF_TYPE type;
    int use_cnt; /* private */
} hwc_cached_buffer_t;

/* api */

/*
 * Return a hwc_cache_buffer if it mapped correctly. Usually this means
 * if it is mapped into Tiler 2D or 1D.
 *
 * Return values:
 * 0    Success. 'cbuf' will be filled with the buffer properties
 * <0   The buffer associated with the layer is not mapped.
 */
int get_cached_buffer(hwc_layer_t *layer, hwc_cached_buffer_t *cbuf);

/*
 * Map a layer's buffer. Usually called if get_cached_buffer() fails.
 * Usually only called if the buffer needs mapping into Tiler-1D, although
 * you can call this for either buffer type - the api doesn't care.
 *
 * As mentioned elsewhere Tiler-1D buffers are treated as a 'cache' so
 * only a limited number of them are mapped into Tiler-1F. This is a
 * least recently used cache - so frequently updated layers will not be
 * constant mapped and remapped (consider 3D games).
 *
 * Return values:
 * 0    Success. The layer's buffer is mapped into Tiler-1D. 
 */
int map_cached_buffer(hwc_layer_t *layer);

/*
 * This doesn't really do anything yet. The cache will unmap buffers dynamically
 * so this is only useful for unloading the HAL
 */
void unmap_cached_buffers(void);

/*
 * Initialise the buffer cache module
 */
void init_cached_buffers(void);

#endif // _HWC_BUFFERS_H
