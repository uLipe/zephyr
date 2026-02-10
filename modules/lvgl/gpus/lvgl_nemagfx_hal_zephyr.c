/*
 * Copyright (c) 2025 Felipe Neves <ryukokki.felipe@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT zephyr_lvgl_gpu_nema

#include <lvgl.h>
#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/sys_io.h>
#include <zephyr/sys/util.h>
#include <zephyr/init.h>
#include <zephyr/sys/sys_heap.h>
#include <soc.h>

#include <nema_sys_defs.h>
#include <nema_core.h>
#include <tsi_malloc.h>

#define NEMA_INST	        0
#define NEMA_NODE		DT_INST(NEMA_INST, zephyr_lvgl_gpu_nema)
#define NEMA_GPU_NODE		DT_PHANDLE(NEMA_NODE, gpu)
#define NEMA_GPU_BASE		DT_REG_ADDR(NEMA_GPU_NODE)
#define NEMA_GPU_IRQN		DT_IRQN(NEMA_GPU_IRQN)
#define NEMA_GPU_ERR_IRQN	(NEMA_GPU_IRQN + 1)
#define NEMA_RING_SIZE		1024

#define NEMA_ITCTRL		0x0F8U/* Interrupt Control Register Offset            */
#define NEMA_CLID		0x148U/* Last Command List Identifier Register Offset */
#define NEMA_BREAKPOINT		0x080U/* Breakpoint Register Offset                   */
#define NEMA_SYS_INTERRUPT	0xff8U/* System Interrupt Register Offset             */
#define NEMA_IT_CLC		0x00000001U/* Command List Complete Interrupt */
#define NEMA_FLAG_CLC		0x00000001U/* Command List Complete Interrupt Flag  */

typedef void (*irq_config_func_t)(const struct device *dev);

struct z_nema_config {
	uint32_t gpu_base_address;
	irq_config_func_t irq_config;
};

struct z_nema_data {
	struct k_sem sync;
	uint32_t last_cl_id;
	nema_ringbuffer_t ring_buffer_str;
	uint8_t nemagfx_pool_mem[CONFIG_LV_Z_NEMA_GFX_HEAP_SIZE];
};

uint32_t nema_reg_read(uint32_t reg)
{
	const struct device *dev = DEVICE_DT_GET(NEMA_NODE);
	struct z_nema_config *dev_cfg = dev->config;

	return sys_read32((mem_addr_t)(dev_cfg->gpu_base_address + reg));
}

void nema_reg_write(uint32_t reg, uint32_t value)
{
	const struct device *dev = DEVICE_DT_GET(NEMA_NODE);
	struct z_nema_config *dev_cfg = dev->config;

	sys_write32(value, (mem_addr_t)(dev_cfg->gpu_base_address + reg));
}

int nema_wait_irq(void)
{
	const struct device *dev = DEVICE_DT_GET(NEMA_NODE);
	struct z_nema_data *dev_data = dev->data;

	return k_sem_take(&dev_data->sync, K_FOREVER);
}

int nema_wait_irq_cl(int cl_id)
{
	const struct device *dev = DEVICE_DT_GET(NEMA_NODE);
	struct z_nema_data *dev_data = dev->data;
	struct z_nema_config *dev_cfg = dev->config;

	while(dev_data->last_cl_id < cl_id) {
		(void)nema_wait_irq();
	}

    return 0;
}

int nema_wait_irq_brk(int brk_id)
{
#ifdef NEMA_BREAKPOINT
	const struct device *dev = DEVICE_DT_GET(NEMA_NODE);
	struct z_nema_data *dev_data = dev->data;
	struct z_nema_config *dev_cfg = dev->config;

	while(nema_reg_read(NEMA_BREAKPOINT) == 0U) {
	        (void)nema_wait_irq();
    	}
#endif
	return 0;
}

void nema_host_free(void *ptr)
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

void * nema_buffer_map(nema_buffer_t *bo)
{
	return bo->base_virt;
}

void nema_buffer_unmap(nema_buffer_t *bo)
{
	ARG_UNUSED(bo);
}

void nema_buffer_destroy(nema_buffer_t *bo)
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

uintptr_t nema_buffer_phys(nema_buffer_t *bo)
{
	return bo->base_phys;
}

void nema_buffer_flush(nema_buffer_t *bo)
{
	ARG_UNUSED(bo);
}

int nema_mutex_lock(int mutex_id)
{
	ARG_UNUSED(mutex_id);

	return 0;
}

int nema_mutex_unlock(int mutex_id)
{
	ARG_UNUSED(mutex_id);

	return 0;
}

void platform_disable_cache(void)
{
	/* stub required otherwise libnema will not link */
}

void platform_invalidate_cache(void)
{
	/* stub required otherwise libnema will not link */
}

int32_t nema_sys_init(void)
{
	const struct device *dev = DEVICE_DT_GET(NEMA_NODE);
	struct z_nema_data *dev_data = dev->data;


	int32_t error_code = tsi_malloc_init_pool_aligned(0, (void *)dev_data->nemagfx_pool_mem,
							(uintptr_t)dev_data->nemagfx_pool_mem,
							CONFIG_LV_Z_NEMA_GFX_HEAP_SIZE, 1, 8);

	dev_data->ring_buffer_str.bo = nema_buffer_create(NEMA_RING_SIZE);
	error_code |= nema_rb_init(&dev_data->ring_buffer_str, 1);

	return error_code;
}


static void z_nema_isr(const void *arg)
{
	const struct device *dev = (const struct device *)arg;
	struct z_nema_data *dev_data = dev->data;
	struct z_nema_config *dev_cfg = dev->config;
	uint32_t isr_flags = nema_reg_read(NEMA_ITCTRL);

  	if (isr_flags & NEMA_FLAG_CLC) {

		isr_flags &= ~(NEMA_FLAG_CLC);
		nema_reg_write(NEMA_ITCTRL, isr_flags);
	    	dev_data->last_cl_id = nema_reg_read(NEMA_CLID);
		k_sem_give(&dev_data->sync);
	}
}

static void z_nema_err_isr(const void *arg)
{
	const struct device *dev = (const struct device *)arg;
	struct z_nema_data *dev_data = dev->data;
	struct z_nema_config *dev_cfg = dev->config;
	k_sem_give(&dev_data->sync);
}

static void z_nema_irq_config_func(const struct device *dev)
{
	ARG_UNUSED(dev);

	IRQ_CONNECT(NEMA_GPU_IRQN, 0, z_nema_isr,
		    DEVICE_DT_INST_GET(NEMA_INST), 0);
	irq_enable(NEMA_GPU_IRQN);

	IRQ_CONNECT(NEMA_GPU_ERR_IRQN, 0, z_nema_err_isr,
		    DEVICE_DT_INST_GET(NEMA_INST), 0);
	irq_enable(NEMA_GPU_ERR_IRQN);
}

static int z_nema_init(const struct device *dev)
{
	struct z_nema_data *dev_data = dev->data;
	struct z_nema_config *dev_cfg = dev->config;

	dev_cfg->irq_config(dev);
	dev_data->last_cl_id = 0;
	k_sem_init(&dev_data->sync, 0, 1);

	return 0;
}

static const struct z_nema_config z_nema_config_0 = {
	.gpu_base_address = NEMA_GPU_BASE,
	.irq_config = z_nema_irq_config_func,
};

static __nocache struct z_nema_data z_nema_data_0;

DEVICE_DT_INST_DEFINE(0,
	      z_nema_init,
	      NULL,
	      &z_nema_data_0,
	      &z_nema_config_0,
	      POST_KERNEL,
	      CONFIG_APPLICATION_INIT_PRIORITY,
	      NULL);
