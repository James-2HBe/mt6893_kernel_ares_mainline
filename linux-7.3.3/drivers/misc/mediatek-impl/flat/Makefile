# 15 个 MTK 私有驱动（4.14 -> 7.3.3 API 适配）+ 31 符号 weak 桩
# 全部 obj-y 内建；头路径用 $(srctree) 相对内核根

obj-y +=
obj-y += clk-mt6873-pg.o
obj-y += helio-dvfsrc-opp.o
obj-y += emimpu.o
obj-y += mtk_clkbuf_hw.o
obj-y += mtk_ppm_main.o
obj-y += mtk_ppm_policy_sys_boost.o
obj-y += mtk_spm_resource_req.o
obj-y += mtk_freqhopping_drv.o
obj-y += topo_ctrl.o
obj-y += upmu_lbat_service_v2.o
obj-y += pmic_lp_api.o
obj-y += pmic.o
obj-y += mtk_battery_intf.o
obj-y += pseudo_m4u_v2.o
obj-y += mtu3_dr.o
obj-y += impl-symbols.o

# 阶段 D：从 4.19 原厂树真移植的子系统实现（覆盖对应 weak 桩）
obj-$(CONFIG_MTK_ARES_REAL_SUBSYS) += mtk_ppm_profile.o
obj-$(CONFIG_MTK_ARES_REAL_SUBSYS) += mtk_ppm_interface.o
obj-$(CONFIG_MTK_ARES_REAL_SUBSYS) += mtk_lp_sysfs.o
obj-$(CONFIG_MTK_ARES_REAL_SUBSYS) += real-subsys-stubs.o

# 注入私有头路径（保真 include 结构 + compat 接线头）
ccflags-y +=
ccflags-y += -I$(srctree)/drivers/misc/mediatek-impl
ccflags-y += -I$(srctree)/drivers/misc/mediatek-impl/compat
ccflags-y += -I$(srctree)/drivers/misc/mediatek-impl/compat/asm
ccflags-y += -I$(srctree)/drivers/misc/mediatek-impl/compat/linux
ccflags-y += -I$(srctree)/drivers/misc/mediatek-impl/compat/linux/usb
ccflags-y += -I$(srctree)/drivers/misc/mediatek-impl/compat/soc
ccflags-y += -I$(srctree)/drivers/misc/mediatek-impl/compat/soc/mediatek
ccflags-y += -I$(srctree)/drivers/misc/mediatek-impl/compat/trace
ccflags-y += -I$(srctree)/drivers/misc/mediatek-impl/compat/trace/events
ccflags-y += -I$(srctree)/drivers/misc/mediatek-impl/drivers
ccflags-y += -I$(srctree)/drivers/misc/mediatek-impl/drivers/misc
ccflags-y += -I$(srctree)/drivers/misc/mediatek-impl/drivers/misc/mediatek
ccflags-y += -I$(srctree)/drivers/misc/mediatek-impl/drivers/misc/mediatek/base
ccflags-y += -I$(srctree)/drivers/misc/mediatek-impl/drivers/misc/mediatek/base/power
ccflags-y += -I$(srctree)/drivers/misc/mediatek-impl/drivers/misc/mediatek/base/power/clkbuf_v1
ccflags-y += -I$(srctree)/drivers/misc/mediatek-impl/drivers/misc/mediatek/base/power/clkbuf_v1/mt6853
ccflags-y += -I$(srctree)/drivers/misc/mediatek-impl/drivers/misc/mediatek/base/power/clkbuf_v1/mt6873
ccflags-y += -I$(srctree)/drivers/misc/mediatek-impl/drivers/misc/mediatek/base/power/clkbuf_v1/mt6885
ccflags-y += -I$(srctree)/drivers/misc/mediatek-impl/drivers/misc/mediatek/pseudo_m4u
ccflags-y += -I$(srctree)/drivers/misc/mediatek-impl/include
ccflags-y += -I$(srctree)/drivers/misc/mediatek-impl/include/dt-bindings
ccflags-y += -I$(srctree)/drivers/misc/mediatek-impl/include/dt-bindings/clock
ccflags-y += -I$(srctree)/drivers/misc/mediatek-impl/include/dt-bindings/memory
ccflags-y += -I$(srctree)/drivers/misc/mediatek-impl/include/include
ccflags-y += -I$(srctree)/drivers/misc/mediatek-impl/include/mach
ccflags-y += -I$(srctree)/drivers/misc/mediatek-impl/include/memory
ccflags-y += -I$(srctree)/drivers/misc/mediatek-impl/include/memory/mediatek
ccflags-y += -I$(srctree)/drivers/misc/mediatek-impl/include/mt-plat
ccflags-y += -I$(srctree)/drivers/misc/mediatek-impl/include/mtk_idle_fs
ccflags-y += -I$(srctree)/drivers/misc/mediatek-impl/include/ppm_v3
