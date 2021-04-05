/*
 * Copyright (c) 2019 Carlo Caione <ccaione@baylibre.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_ARCH_ARM64_INCLUDE_OFFSETS_SHORT_ARCH_H_
#define ZEPHYR_ARCH_ARM64_INCLUDE_OFFSETS_SHORT_ARCH_H_

#include <offsets.h>

#ifdef CONFIG_USERSPACE
#define _thread_offset_to_priv_stack_start \
	(___thread_t_arch_OFFSET + ___thread_arch_t_priv_stack_start_OFFSET)
#endif

#endif /* ZEPHYR_ARCH_ARM64_INCLUDE_OFFSETS_SHORT_ARCH_H_ */
