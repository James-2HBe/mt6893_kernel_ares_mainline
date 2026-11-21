/* SPDX-License-Identifier: GPL-2.0 */
/*
 * compat/linux/dma-iommu.h — 7.3 主线已删除的旧 dma-iommu 适配层兼容桩。
 *
 * 4.14 MTK 私有驱动（pseudo_m4u.c / pseudo_m4u_v2.c）直接引用了 7.3 已移除的
 * 旧 dma-iommu 符号（struct dma_iommu_mapping / to_dma_iommu_mapping /
 * iommu_dma_map_sg / iommu_dma_unmap_sg）。本桩仅用于让 impl-adapted 的 21 个
 * 驱动达到「编译期自包含」（-fsyntax-only 通过），运行期由真实平台实现覆盖。
 */
#ifndef _COMPAT_LINUX_DMA_IOMMU_H
#define _COMPAT_LINUX_DMA_IOMMU_H

#include <linux/types.h>
#include <linux/device.h>
#include <linux/scatterlist.h>
#include <linux/spinlock.h>
#include <linux/iommu.h>

struct iommu_domain;

struct dma_iommu_mapping {
	unsigned long		base;
	unsigned long		bitmap_size;
	unsigned int		bits;
	unsigned int		nr_bitmaps;
	unsigned long		*bitmaps;
	unsigned int		extensions;
	struct iommu_domain	*domain;
	spinlock_t		lock;
};

static inline struct dma_iommu_mapping *to_dma_iommu_mapping(struct device *dev)
{
	return NULL;
}

/* 4.14 MTK 私有调用为 4 参 (dev, sgl, nents, prot)；7.3 已删此符号，
 * 桩签名按 4.14 真实调用对齐，使 -fsyntax-only 通过。 */
dma_addr_t iommu_dma_map_sg(struct device *dev, struct scatterlist *sg,
			    int nents, int prot);
void iommu_dma_unmap_sg(struct device *dev, struct scatterlist *sg,
			int nents, int prot, unsigned long attrs);

#endif /* _COMPAT_LINUX_DMA_IOMMU_H */
