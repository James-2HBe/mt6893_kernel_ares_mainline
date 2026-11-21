/* SPDX-License-Identifier: GPL-2.0 */
/*
 * compat/asm/dma-iommu.h — arm64 目标树无此头（仅 arch/arm 有），
 * 4.14 MTK 驱动仍 #include <asm/dma-iommu.h>。复用 linux 桩，避免重复定义。
 */
#ifndef _COMPAT_ASM_DMA_IOMMU_H
#define _COMPAT_ASM_DMA_IOMMU_H

#include <linux/dma-iommu.h>

#endif /* _COMPAT_ASM_DMA_IOMMU_H */
