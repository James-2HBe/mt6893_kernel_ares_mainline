/**
 * MT6893 GPIO platform info.
 * MTK 通用 GPIO 寄存器布局，MaxPin=0xdc(220) 来自 mt6893.dtsi gpio-ranges。
 */
#include <Library/GpioImplLib.h>

MTK_GPIO_PLATFORM_INFO gPlatformInfo = {
  .SetOffset     = 0x4,
  .ResetOffset   = 0x8,
  .DirOffset     = 0x0,
  .DataOutOffset = 0x100,
  .DataInOffset  = 0x200,
  .ModeOffset    = 0x300,
  .MaxPin        = 220,
};
