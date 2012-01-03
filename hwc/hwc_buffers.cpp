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
#include <assert.h>
#include <stdlib.h>

#include <hardware/hwcomposer.h>
#include <cutils/native_handle.h>

extern "C" { 
#include <memmgr.h>
}

#include "hwc_priv.h"
#include "hwc_buffers.h"

#define PAGE_MASK (~4095)
#define PAGE_ALIGN(x) (((x) + ~PAGE_MASK) & PAGE_MASK)

#define MAP_CACHE_MAX_SZ 6
static hwc_cached_buffer_t g_map_cache[MAP_CACHE_MAX_SZ];
static hwc_cached_buffer_t *g_head = &g_map_cache[0];

/*
 * Find a matching buffer in the list according to the layer
 * buffer handle
 */
hwc_cached_buffer_t* find_buf(hwc_layer_t *layer)
{
    IMG_native_handle_t *hndl = (IMG_native_handle_t*)layer->handle;
    hwc_cached_buffer_t* cbuf2 = g_head;
    hwc_cached_buffer_t* found = NULL;
    do {
        if ((cbuf2->layer_buffer) &&
            (hndl->vptr == cbuf2->layer_buffer->vptr)) {
            found = cbuf2;
            break;
        }
        cbuf2 = cbuf2->l.next;
    } while (cbuf2);
    return found;
}

/*
 * The least recently used buffer is at the end of the list
 */
static hwc_cached_buffer_t* get_lru(void)
{
    hwc_cached_buffer_t *ptr = g_head;
    while (ptr->l.next != NULL)
        ptr = ptr->l.next;
    return ptr;
}

/*
 * Make the cached buffer passed in the the most recently
 * used buffer i.e. the head of the list
 */
static void swap_head(hwc_cached_buffer_t *buf)
{
    if (buf == g_head) {
        return;
    }
    hwc_cached_buffer_t *prev = g_head;
    while (prev->l.next != buf) {
        prev = prev->l.next;
    }

    hwc_cached_buffer_t *oldhead = g_head;
    prev->l.next = buf->l.next;
    buf->l.next = oldhead;
    g_head = buf;
}

int get_cached_buffer(hwc_layer_t *layer, hwc_cached_buffer_t *cbuf)
{
    IMG_native_handle_t *hndl = (IMG_native_handle_t*)layer->handle;

    if (hndl->iFormat == HAL_PIXEL_FORMAT_NV12) {
        cbuf->layer_buffer = hndl;
        cbuf->mapped_ptr = (void*) hndl->vptr;
        cbuf->type = BUF_TILER;
        return 0;
    }

    int rv = -ENOENT;
    hwc_cached_buffer_t* found = find_buf(layer);
    if (found) {
        swap_head(found);
        *cbuf = *found;
        rv = 0;
    } 
    return rv;
}

int map_cached_buffer(hwc_layer_t *layer)
{
    IMG_native_handle_t* hndl;
    int rv = 0;

    if (!layer)
        return -ENOENT;

    hndl = (IMG_native_handle_t*) layer->handle;
    if (hndl->iFormat == HAL_PIXEL_FORMAT_NV12) {
        return 0;    /* Don't need to map or cache */
    }

    hwc_cached_buffer_t* cbuf = get_lru();
    if (cbuf->layer_buffer) {
        int err = MemMgr_UnMap(cbuf->mapped_ptr);
        LOGE_IF(err, "Unable to unmap buffer %p", cbuf->layer_buffer->vptr);
        bzero(cbuf, sizeof(cbuf));
    }

    void *buf = hndl->vptr;
    __u32 bufi = (__u32) buf;
    __u32 st = bufi & PAGE_MASK;
    MemAllocBlock block[1];
    memset(&block, 0, sizeof(block));

    block[0].pixelFormat = PIXEL_FMT_PAGE;
    block[0].dim.len = PAGE_ALIGN(bufi - st + hndl->m_size);
    block[0].ptr = (void *)st;

    cbuf->mapped_ptr = MemMgr_Map(block, 1);

    if (cbuf->mapped_ptr == NULL) {
        LOGE("Tiler1D mapping failed %p", (void*) hndl->vptr);
        bzero(cbuf, sizeof(cbuf));
        rv = -EINVAL;
        goto end;
    }

    cbuf->type = BUF_USER;
    cbuf->layer_buffer = hndl;
    cbuf->use_cnt = 1;

    LOGV("Buffer %p cached with tiler addr %p", hndl->vptr, cbuf->mapped_ptr);
end:
    return rv;
}

void unmap_cached_buffers(void)
{
}

void init_cached_buffers(void)
{
    const int tail = MAP_CACHE_MAX_SZ-1;
    for (int idx=0; idx < tail; idx++) {
        g_map_cache[idx].l.next = &g_map_cache[idx+1];
    }
    g_map_cache[tail].l.next = NULL;
}
