/*
 * Copyright 2025 Felipe Neves
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stm32n6xx_hal.h>
#include <stm32n6xx_ll_icache.h>
#include <soc.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/clock_control.h>
#include <zephyr/irq.h>
#include <zephyr/sys/sys_io.h>

static K_SEM_DEFINE(gpu_sync, 0, 1);
static GPU2D_HandleTypeDef board_gpu2d_handle;
static uint32_t last_cl_id = 0;

void HAL_GPU2D_CommandListCpltCallback(GPU2D_HandleTypeDef * hgpu2d, uint32_t cmd)
{
    ARG_UNUSED(hgpu2d);
 
    last_cl_id = cmd;
}

static void board_gpu2d_isr(const void *arg)
{
    HAL_GPU2D_IRQHandler(&board_gpu2d_handle);
    k_sem_give(&gpu_sync);
}

static void board_gpu2d_error_isr(const void *arg)
{
    HAL_GPU2D_ER_IRQHandler(&board_gpu2d_handle);
}

static int board_gpu2d_init(void)
{
    __HAL_RCC_GPU2D_CLK_ENABLE();

    LL_ICACHE_Enable();
    LL_ICACHE_SetMode(LL_ICACHE_1WAY);

    board_gpu2d_handle.Instance = GPU2D;
    HAL_GPU2D_Init(&board_gpu2d_handle);
    IRQ_CONNECT(GPU2D_IRQn, 0, board_gpu2d_isr, NULL, 0);
    IRQ_CONNECT(GPU2D_ER_IRQn, 0, board_gpu2d_error_isr, NULL, 0);
    irq_enable(GPU2D_IRQn);
    irq_enable(GPU2D_ER_IRQn);
    irq_enable(ICACHE_IRQn);

    return 0;
}
SYS_INIT(board_gpu2d_init, POST_KERNEL, CONFIG_APPLICATION_INIT_PRIORITY);

/**
 * LOW level NEMA required functions go here for now 
 */
uint32_t nema_reg_read(uint32_t reg)
{
    return sys_read32((board_gpu2d_handle.Instance) + reg);
}

void nema_reg_write(uint32_t reg, uint32_t value)
{
    sys_write32(value, (board_gpu2d_handle.Instance) + reg);
}

int nema_wait_irq(void)
{
    return k_sem_take(&gpu_sync, K_FOREVER);
}

int nema_wait_irq_cl(int cl_id)
{
    while(last_cl_id < cl_id) {
        (void)nema_wait_irq();
    }

    return 0;
}

int nema_wait_irq_brk(int brk_id)
{
    while(nema_reg_read(GPU2D_BREAKPOINT) == 0U) {
        (void)nema_wait_irq();
    }

    return 0;
}
