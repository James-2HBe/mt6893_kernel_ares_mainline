/* SPDX-License-Identifier: GPL-2.0 */
/*
 * compat_selftest.c — 验证 mtk_wifi_compat.h 在 Linux 7.3.x 真实头文件下可编译。
 * 用原厂私有栈里的各类「旧 API 典型用法」逐一调用，若 -fsyntax-only 通过，
 * 说明 compat 头对每个符号的补齐与主线 7.3.3 的类型/签名一致。
 *
 * 编译：由 tools/compat_compile_test.sh 用主线 KBUILD_CFLAGS 驱动。
 * 这不是功能测试，只是「API 兼容层自洽性」的编译期证明。
 */
#include "mtk_wifi_compat.h"

/* A. set_fs 家族 + kernel_read（vfs_read 由 adapt 改，这里直接测新的） */
static void test_setfs(struct file *f, void *buf, size_t n)
{
	mm_segment_t oldfs = get_fs();
	loff_t pos = 0;
	set_fs(get_ds());
	kernel_read(f, buf, n, &pos);
	set_fs(oldfs);
	(void)USER_DS;
}

/* B. struct timeval + do_gettimeofday */
static unsigned long long test_timeofday(void)
{
	struct timeval tv;
	do_gettimeofday(&tv);
	return (unsigned long long)tv.tv_sec * 1000000ULL + (unsigned long long)tv.tv_usec;
}

/* C. VERIFY_* */
static void test_verify(void)
{
	int a = VERIFY_READ;
	int b = VERIFY_WRITE;
	(void)a; (void)b;
}

/* D. strlcpy */
static void test_strlcpy(char *dst, const char *src)
{
	strlcpy(dst, src, 16);
}

/* E. dma_zalloc_coherent */
static void *test_dma(struct device *dev, dma_addr_t *dh)
{
	return dma_zalloc_coherent(dev, 4096, dh, GFP_KERNEL);
}

/* F. create_proc_entry */
static struct proc_dir_entry *test_proc(void)
{
	return create_proc_entry("mtk_test", 0664, NULL);
}

/* 汇总，避免未使用告警 */
void mtk_compat_selftest(void)
{
	(void)test_setfs;
	(void)test_timeofday;
	(void)test_verify;
	(void)test_strlcpy;
	(void)test_dma;
	(void)test_proc;
}
