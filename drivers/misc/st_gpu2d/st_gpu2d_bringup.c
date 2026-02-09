/* Copyright (c) 2025 Felipe Neves <ryukokki.felipe@gmail.com>.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/reset.h>
#include <zephyr/drivers/clock_control.h>
#include <zephyr/drivers/clock_control/stm32_clock_control.h>

#include <stm32_ll_bus.h>
#include <stm32_ll_pwr.h>
#include <stm32_ll_icache.h>
#include <cmsis_core.h>

#define DT_DRV_COMPAT st_stm32_gpu2d

/* even though this can be added to the soc level we prefer to re-define it here
 * to make the gpu2d bring-up self contained
 */
#define RIF_MASTER_CID1_SEC_PRIV(device)	\
	do {										\
		RIMC_MasterConfig_t rimc = {						\
			.MasterCID = RIF_CID_1,						\
			.SecPriv = RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV,		\
		};									\
		HAL_RIF_RIMC_ConfigMasterAttributes(RIF_MASTER_INDEX_##device, &rimc);	\
	} while (0)

#define RIF_SLAVE_SEC_PRIV(device)	\
	HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_##device,		\
					      RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV)


struct stm32_gpu2d_cfg {
	struct stm32_pclken pclken;
	const struct reset_dt_spec reset;
};

static int stm32_gpu2d_init(const struct device *dev)
{
	const struct stm32_gpu2d_cfg *cfg = dev->config;
	const struct device *clk = DEVICE_DT_GET(STM32_CLOCK_CONTROL_NODE);

	/* Make the security system aware of the GPU2D and Texture cache */
	RIF_MASTER_CID1_SEC_PRIV(GPU2D);
	RIF_SLAVE_SEC_PRIV(GPU2D);
	RIF_SLAVE_SEC_PRIV(ICACHE);

	if (!device_is_ready(clk)) {
		return -ENODEV;
	}

	if (clock_control_on(clk, (clock_control_subsys_t)&cfg->pclken) != 0) {
		return -EIO;
	}

	if (!device_is_ready(cfg->reset.dev)) {
		return -ENODEV;
	}

	reset_line_toggle_dt(&cfg->reset);

	return 0;
}

static const struct stm32_gpu2d_cfg stm32_gpu2d_cfg_0 = {
	.pclken = STM32_CLOCK_INFO(0, DT_NODELABEL(gpu2d)),
	.reset = RESET_DT_SPEC_GET_BY_IDX(DT_NODELABEL(gpu2d), 0),
};

DEVICE_DT_DEFINE(DT_NODELABEL(gpu2d),
		stm32_gpu2d_init,
		NULL,
		NULL,
		stm32_gpu2d_cfg_0,
		POST_KERNEL,
		CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,
		NULL);
