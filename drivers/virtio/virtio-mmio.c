/*
 * Copyright (c) 2021 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include <zephyr/kernel.h>
#include <openamp/open_amp.h>
#include <openamp/virtqueue.h>
#include <openamp/virtio.h>
#include <metal/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/device.h>
#include <zephyr/drivers/virtio/virtio.h>
#include <zephyr/linker/linker-defs.h>
#include <zephyr/sys/device_mmio.h>

#if defined(CONFIG_VIRTIO_MMIO_USE_IVSHMEM)
#include <zephyr/drivers/virtualization/ivshmem.h>

#define VIRTIO_MMIO_IVSHMEM_HVL_SHM_OFFSET 0x8000
#define VIRTIO_MMIO_IVSHMEM_VRING_SHM_OFFSET (VIRTIO_MMIO_IVSHMEM_HVL_SHM_OFFSET + 0x80000)
#endif

#define DT_DRV_COMPAT virtio_mmio

struct virtio_mmio_config {
    void (*irq_config)(const struct device *dev);
};

#define DEV_CFG(dev) ((struct virtio_mmio_config *)(dev->config))
#define DEV_DATA(dev) ((struct virtio_mmio_device *)(dev->data))

static int virtio_mmio_init(const struct device *dev);
struct virtio_device* virtio_mmio_get_virtio_device(const struct device *dev) {
    return &DEV_DATA(dev)->vdev;
}

static const struct virtio_driver_api virtio_mmio_api = {
    .get_virtio_device = virtio_mmio_get_virtio_device,
};

static struct k_heap hvl_shmem_k_heap;

#if defined(CONFIG_VIRTIO_MMIO_USE_IVSHMEM)
static struct k_heap vring_shmem_k_heap;

struct virtqueue * virtio_mmio_virtqueue_alloc(int n, int align)
{
    struct virtqueue *vq = k_heap_alloc(&vring_shmem_k_heap, sizeof(struct virtqueue), K_NO_WAIT);
    if(!vq)
        return NULL;

    /* Allocate the VRING buffer: */
    char *vrbuf = k_heap_aligned_alloc(&vring_shmem_k_heap,
                                        align,
                                        VRING_SIZE(n, align),
                                        K_NO_WAIT);
    if(!vrbuf) {
        k_heap_free(&vring_shmem_k_heap, vq);
        return NULL;
    }

    memset(vrbuf, 0, VRING_SIZE(n, align));

    /* populate the vq structure :*/
    vq->vq_nentries = n;
    vq->vq_ring.desc = (void *)vrbuf;
    vq->vq_ring.avail = (void *)((unsigned long)vrbuf +
					n * sizeof(struct vring_desc));
    vq->vq_ring.used = (void *)((unsigned long)vrbuf +
					((n * sizeof(struct vring_desc) +
					(n + 1) * sizeof(uint16_t) + align - 1) & ~(align - 1)));
    vq->vq_queued_cnt = 0;
    vq->vq_free_cnt = n;

    return vq;
}


void virtio_mmio_vring_free(struct virtqueue * vq)
{
    k_heap_free(&vring_shmem_k_heap, vq->vq_ring.desc);
    k_heap_free(&vring_shmem_k_heap, vq);
}

#endif

void virtio_mmio_shm_pool_init(void *mem, size_t size)
{
    static int init = 0;
#if !defined(CONFIG_VIRTIO_MMIO_USE_IVSHMEM)
    if (!init) {
        k_heap_init(&hvl_shmem_k_heap, mem, size);
        init = 1;
    }
#else
    ARG_UNUSED(mem);
    ARG_UNUSED(size);

    const struct device *ivshmem_dev =
        DEVICE_DT_GET_ANY(qemu_ivshmem);

    uintptr_t ivshmem_base;
    ivshmem_get_mem(ivshmem_dev, &ivshmem_base);

    void *hvl_shmem_area = (void *) (DT_REG_ADDR(DT_NODELABEL(vmram_ivshmem1)) + ivshmem_base);
    void *hvl_vring_area = (void *) (DT_REG_ADDR(DT_NODELABEL(vring_ivshmem)) + ivshmem_base);

    if (!init) {
        k_heap_init(&hvl_shmem_k_heap, hvl_shmem_area, DT_REG_SIZE(DT_NODELABEL(vmram_ivshmem1)));
        k_heap_init(&vring_shmem_k_heap, hvl_vring_area, DT_REG_SIZE(DT_NODELABEL(vring_ivshmem)));
        init = 1;
    }
#endif
}

void* virtio_mmio_shm_pool_alloc(size_t size)
{
    return k_heap_alloc(&hvl_shmem_k_heap, size, K_NO_WAIT);
}

void virtio_mmio_shm_pool_free(void *ptr)
{
    k_heap_free(&hvl_shmem_k_heap, ptr);
}

#define CREATE_VIRTIO_MMIO_DEVICE(inst)                              \
    static struct virtio_mmio_device virtio_mmio_data_##inst = {       \
        .vdev = { \
            .priv = &virtio_mmio_data_##inst, \
        }, \
        .irq = DT_INST_IRQN(inst),                               \
        .device_mode = !DT_PROP(DT_INST(inst, DT_DRV_COMPAT), driver_mode),   \
        .cfg_mem = { \
            .base = (uint8_t *)DT_INST_REG_ADDR(inst),                   \
            .size = (unsigned int)DT_INST_REG_SIZE(inst),              \
        }, \
        .shm_mem = { \
            .base = (uint8_t *)DT_REG_ADDR(DT_PHANDLE(DT_INST(inst, DT_DRV_COMPAT), memory_region)), \
            .size = (unsigned int)DT_REG_SIZE(DT_PHANDLE(DT_INST(inst, DT_DRV_COMPAT), memory_region)), \
        }, \
        .shm_device = {                                      \
            .name = DT_PROP(DT_INST(inst, DT_DRV_COMPAT), label),                         \
            .bus = NULL,                                     \
            .num_regions = 2,                                \
            {                                                \
                {                                            \
                    .virt       = (void *) NULL,         \
                    .physmap    = NULL,                  \
                    .size       = 0,                     \
                    .page_shift = (uintptr_t)0xffffffffffffffff,    \
                    .page_mask  = (uintptr_t)0xffffffffffffffff,    \
                    .mem_flags  = 0,                     \
                    .ops        = { NULL },              \
                },                                           \
                {                                            \
                    .virt       = (void *) NULL,         \
                    .physmap    = NULL,                  \
                    .size       = 0,                     \
                    .page_shift = (uintptr_t)0xffffffffffffffff,    \
                    .page_mask  = (uintptr_t)0xffffffffffffffff,    \
                    .mem_flags  = 0,                     \
                    .ops        = { NULL },              \
                },                                           \
            },                                               \
            .node = { NULL },                                \
            .irq_num = 0,                                    \
            .irq_info = NULL                                 \
        }                                                    \
    };                                                               \
    static void irq_config_##inst(const struct device *dev)          \
    {\
        IRQ_CONNECT(DT_INST_IRQ(inst, irq),\
                    DT_INST_IRQ(inst, priority),\
                    virtio_mmio_isr,\
                    &virtio_mmio_data_##inst.vdev,\
                    0);\
        irq_enable(DT_INST_IRQ(inst, irq));\
    }\
    static struct virtio_mmio_config virtio_mmio_cfg_##inst = {      \
        .irq_config = irq_config_##inst,                             \
    };                                                               \
    DEVICE_DT_INST_DEFINE(inst,                                      \
                          virtio_mmio_init,                          \
                          NULL,                                      \
                          &virtio_mmio_data_##inst,                  \
                          &virtio_mmio_cfg_##inst,                   \
                          POST_KERNEL,                               \
                          CONFIG_APPLICATION_INIT_PRIORITY,          \
                          &virtio_mmio_api);

DT_INST_FOREACH_STATUS_OKAY(CREATE_VIRTIO_MMIO_DEVICE)

static int virtio_mmio_init(const struct device *dev)
{
    uintptr_t sram0_addr;
    uintptr_t virt_mem_ptr;
    uintptr_t cfg_mem_ptr;
    struct virtio_mmio_device *vmdev = DEV_DATA(dev);

#if !defined(CONFIG_VIRTIO_MMIO_USE_IVSHMEM)

    /* Map config */
    cfg_mem_ptr = (uintptr_t)vmdev->cfg_mem.base;

#if defined(CONFIG_MMU)
    device_map(&cfg_mem_ptr, (uintptr_t)vmdev->cfg_mem.base,
               vmdev->cfg_mem.size, K_MEM_CACHE_NONE);
#endif

    //TODO: rework detection of full virtualization mode
    //For now, guest mode implies that the memory-region configured in the DTS
    //is the sram0 memory region, e.g. memory-region = <&sram0>;

    virt_mem_ptr = (uintptr_t)vmdev->shm_mem.base;
    sram0_addr = (uintptr_t)DT_REG_ADDR(DT_NODELABEL(sram0));

    if (sram0_addr == (uintptr_t)vmdev->shm_mem.base) {
        //memory already mapped
        virt_mem_ptr = (uintptr_t)Z_MEM_VIRT_ADDR((uintptr_t)vmdev->shm_mem.base);
    } else {
        /* Map dedicated mem region */
#if defined(CONFIG_MMU)
        device_map(&virt_mem_ptr, (uintptr_t)vmdev->shm_mem.base,
                    vmdev->shm_mem.size, K_MEM_CACHE_NONE);
#endif
    }

#else /* !CONFIG_VIRTIO_MMIO_USE_IVSHMEM*/
    const struct device *ivshmem_dev =
		DEVICE_DT_GET_ANY(qemu_ivshmem);

    uintptr_t ivshmem_base;

    size_t size = ivshmem_get_mem(ivshmem_dev, &ivshmem_base);

    cfg_mem_ptr = (uintptr_t)vmdev->cfg_mem.base + ivshmem_base;
    virt_mem_ptr = (uintptr_t)vmdev->shm_mem.base + ivshmem_base;

    /* make the physmap 1:1 IVSHMEM already resolve the address:*/
    vmdev->cfg_mem.base = (void *)cfg_mem_ptr;
    vmdev->shm_mem.base = (void *)virt_mem_ptr;

    /* excludes config space that resides in the same address space */
    vmdev->shm_mem.size = size - 512;

    (void)sram0_addr;
#endif /* CONFIG_VIRTIO_MMIO_USE_IVSHMEM*/

    if (virtio_mmio_device_init(vmdev, virt_mem_ptr, cfg_mem_ptr, (void *)dev))
        return -1;

    metal_log(METAL_LOG_DEBUG, "device %s @%p\n", dev->name, dev);
    metal_log(METAL_LOG_DEBUG, "iobase %lx\n", cfg_mem_ptr);

#if !defined(CONFIG_VIRTIO_MMIO_USE_IVSHMEM)
    DEV_CFG(dev)->irq_config(dev);
#endif

    return 0;
}
