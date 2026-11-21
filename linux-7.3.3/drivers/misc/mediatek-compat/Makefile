# MT6893 4.14 -> 7.3.3 API 适配层：函数级真实现 + 弱符号安全兜底
# 全部 obj-y 内建，确保符号进入 vmlinux/System.map（打包 boot.img 无需模块）
obj-$(CONFIG_MTK_COMPAT_FRAMEWORK_SYMBOLS) += framework-symbols.o
obj-$(CONFIG_MTK_COMPAT_PLATFORM_BRIDGE)   += platform-bridge.o
obj-$(CONFIG_MTK_COMPAT_PLATFORM_SYMBOLS)  += platform-symbols.o
