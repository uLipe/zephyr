/*
 * Copyright (c) 2025 Felipe Neves <ryukokki.felipe@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/irq.h>
#include <zephyr/sys/sys_io.h>
#include <zephyr/cache.h>

#include <nema_sys_defs.h>
#include <nema_core.h>
#include <tsi_malloc.h>
#include <assert.h>
#include <string.h>
#include <lvgl.h>

#define RING_SIZE                      1024 /* Ring Buffer Size in byte */
#if defined(LV_NEMA_GFX_MAX_RESX) && defined(LV_NEMA_GFX_MAX_RESY)
    #define NEMAGFX_MEM_POOL_SIZE          ((LV_NEMA_GFX_MAX_RESX * LV_NEMA_GFX_MAX_RESY) + 10240)
#else
    /* LV_USE_NEMA_VG is 0 so masking/stencling memory is not needed. */
    #define NEMAGFX_MEM_POOL_SIZE          10240
#endif

static uint8_t nemagfx_pool_mem[NEMAGFX_MEM_POOL_SIZE]; /* NemaGFX memory pool */
static nema_ringbuffer_t ring_buffer_str;

int32_t nema_sys_init(void)
{
    int error_code = 0;

    /* Initialise Mem Space */
    error_code = tsi_malloc_init_pool_aligned(0, (void *)nemagfx_pool_mem, (uintptr_t)nemagfx_pool_mem,
                                              NEMAGFX_MEM_POOL_SIZE, 1, 8);
    LV_ASSERT(error_code == 0);

    /* Allocate ring_buffer memory */
    ring_buffer_str.bo = nema_buffer_create(RING_SIZE);
    LV_ASSERT(ring_buffer_str.bo.base_virt);

    /* Initialize Ring Buffer */
    error_code = nema_rb_init(&ring_buffer_str, 1);
    if(error_code < 0) {
        return error_code;
    }

    return error_code;
}


void nema_host_free(void * ptr)
{
    tsi_free(ptr);
}

void * nema_host_malloc(unsigned size)
{
    return tsi_malloc(size);
}

nema_buffer_t nema_buffer_create(int size)
{
    nema_buffer_t bo;
    lv_memset(&bo, 0, sizeof(bo));
    bo.base_virt = tsi_malloc(size);
    bo.base_phys = (uint32_t)bo.base_virt;
    bo.size      = size;
    LV_ASSERT_MSG(bo.base_virt != 0, "Unable to allocate memory in nema_buffer_create");

    return bo;
}

nema_buffer_t nema_buffer_create_pool(int pool, int size)
{
    LV_UNUSED(pool);

    return nema_buffer_create(size);
}

void * nema_buffer_map(nema_buffer_t * bo)
{
    return bo->base_virt;
}

void nema_buffer_unmap(nema_buffer_t * bo)
{
    LV_UNUSED(bo);
}

void nema_buffer_destroy(nema_buffer_t * bo)
{
    if(bo->fd == -1) {
        return; /* Buffer weren't allocated! */
    }

    tsi_free(bo->base_virt);

    bo->base_virt = (void *)0;
    bo->base_phys = 0;
    bo->size      = 0;
    bo->fd        = -1; /* Buffer not allocated */
}

uintptr_t nema_buffer_phys(nema_buffer_t * bo)
{
    return bo->base_phys;
}

void nema_buffer_flush(nema_buffer_t * bo)
{
    LV_UNUSED(bo);
}

int nema_mutex_lock(int mutex_id)
{
    int retval = 0;

    LV_UNUSED(mutex_id);

    return retval;
}

int nema_mutex_unlock(int mutex_id)
{
    int retval = 0;

    LV_UNUSED(mutex_id);

    return retval;
}

void platform_disable_cache(void)
{
    sys_cache_data_disable();
}

void platform_invalidate_cache(void)
{
    sys_cache_data_flush_and_invd_all();
}
