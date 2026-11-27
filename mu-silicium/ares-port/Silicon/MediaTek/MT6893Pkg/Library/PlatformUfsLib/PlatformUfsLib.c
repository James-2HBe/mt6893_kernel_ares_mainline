/* SPDX-License-Identifier: BSD-2-Clause-Patent */
/*
 * MT6893 (Dimensity 1200) UFS platform hooks for the reused Samsung UfsDxe
 * core.  Register bases / SiP commands are cross-checked against the
 * mainline Linux 7.3.3 tree:
 *   drivers/ufs/host/ufs-mediatek.c
 *   drivers/ufs/host/ufs-mediatek-sip.h
 *   arch/arm64/boot/dts/mediatek/mt6893.dtsi (ufshci@11270000)
 */

#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/TimerLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/ArmSmcLib.h>
#include <Library/UfsHostBridge.h>
#include "PlatformUfsLib.h"

/* ---- MT6893 UFS hardware (from mt6893.dtsi) ---------------------------- */
#define MT6893_UFSHCI_BASE        0x11270000UL
#define MT6893_UFSHCI_SIZE        0x2300UL
#define MT6893_UFS_MPHY_BASE      0x11FA0000UL
#define MT6893_UFS_MPHY_SIZE      0xC000UL

/* MediaTek SiP SMC command: MTK_SIP_SMC_CMD(0x276) = 0xC2000276 */
#define MTK_SIP_UFS_CONTROL       0xC2000276UL

/* UFS SiP sub-commands (BIT(n) in Linux ufs-mediatek-sip.h) */
#define UFS_SIP_VA09_PWR_CTRL     (1U << 0)
#define UFS_SIP_DEVICE_RESET      (1U << 1)
#define UFS_SIP_CRYPTO_CTRL       (1U << 2)
#define UFS_SIP_REF_CLK_NOTIFY    (1U << 3)
#define UFS_SIP_SRAM_PWR_CTRL     (1U << 5)
#define UFS_SIP_DEVICE_PWR_CTRL   (1U << 7)
#define UFS_SIP_MPHY_CTRL         (1U << 8)
#define UFS_SIP_MTCMOS_CTRL       (1U << 9)

#define UFS_SIP_STAGE_PRE_CHANGE  0
#define UFS_SIP_STAGE_POST_CHANGE 1

/* UFS_MTK_SIP_MPHY_CTRL operations */
#define UFS_MPHY_BACKUP           0
#define UFS_MPHY_RESTORE          1

/* ---- SiP SMC wrapper --------------------------------------------------- */
STATIC
VOID
MtkUfsSip (
  IN UINTN  SubCmd,
  IN UINTN  Arg1,
  IN UINTN  Arg2,
  IN UINTN  Arg3
  )
{
  ARM_SMC_ARGS  Args;

  ZeroMem (&Args, sizeof (Args));
  Args.Arg0 = MTK_SIP_UFS_CONTROL;
  Args.Arg1 = SubCmd;
  Args.Arg2 = Arg1;
  Args.Arg3 = Arg2;
  Args.Arg4 = Arg3;
  ArmCallSmc (&Args);
}

/* ---- Board init: the one place that knows MT6893 register layout ------- */
EFI_STATUS
UfsBoardInit (
  struct UfsHost  *Ufs
  )
{
  DEBUG ((DEBUG_INFO, "UFS: MT6893 board init\n"));

  if (Ufs == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  /* UFSHCI register window + MPHY. */
  Ufs->IoAddr      = (VOID *)(UINTN)MT6893_UFSHCI_BASE;
  Ufs->VsAddr      = (VOID *)(UINTN)MT6893_UFSHCI_BASE;  /* VS hooks ignore this */
  Ufs->UniProAddr  = (VOID *)(UINTN)(MT6893_UFSHCI_BASE + 0x1000);
  Ufs->UfsPaddr    = (VOID *)(UINTN)MT6893_UFSHCI_BASE;
  Ufs->PhyPma      = (VOID *)(UINTN)MT6893_UFS_MPHY_BASE;
  Ufs->DevPwrAddr  = NULL;   /* VCC from fixed regulator (vcc-fixed-regulator) */
  Ufs->PhyIsoAddr  = NULL;   /* PHY isolation handled by SiP */
  Ufs->DevPwrShift = 0;

  /* 2 lanes / HS-Gear4 as described in mt6893.dtsi */
  Ufs->MclkRate = 166000000UL;
  Ufs->GearMode = 4;

  /*
   * The boot chain (BL2/ATF) generally leaves UFS MTCMOS on.  Still send
   * the standard SiP notifications so the ATF UFS handler is in a known
   * state.  These are no-ops on ATF builds without the UFS SiP service.
   */
  MtkUfsSip (UFS_SIP_MTCMOS_CTRL, 1, 0, 0);        /* MTCMOS on  */
  MicroSecondDelay (200);
  MtkUfsSip (UFS_SIP_REF_CLK_NOTIFY, 1, UFS_SIP_STAGE_PRE_CHANGE, 0);
  MicroSecondDelay (200);
  MtkUfsSip (UFS_SIP_REF_CLK_NOTIFY, 1, UFS_SIP_STAGE_POST_CHANGE, 0);
  MicroSecondDelay (500);

  DEBUG ((DEBUG_INFO, "UFS: MT6893 board init done (io=0x%lx phy=0x%lx gear=%d)\n",
          (UINTN)Ufs->IoAddr, (UINTN)Ufs->PhyPma, Ufs->GearMode));
  return EFI_SUCCESS;
}

/* ---- Calibration hooks ------------------------------------------------- */
/* MT6893 PHY calibration is performed by ATF / the UFS vendor driver; the
 * UEFI stage only needs the link to come up through standard UFSHCI UIC
 * commands, so these hooks intentionally succeed without touching hardware.
 */
UfsCalError
UfsCalInit (struct UfsCalParam *p)
{
  return UFS_CAL_NO_ERROR;
}

UfsCalError
UfsCalPreLink (struct UfsCalParam *p)
{
  return UFS_CAL_NO_ERROR;
}

UfsCalError
UfsCalPostLink (struct UfsCalParam *p)
{
  return UFS_CAL_NO_ERROR;
}

UfsCalError
UfsCalPrePmc (struct UfsCalParam *p)
{
  return UFS_CAL_NO_ERROR;
}

UfsCalError
UfsCalPostPmc (struct UfsCalParam *p)
{
  return UFS_CAL_NO_ERROR;
}

UINT8
UfsCalGetTargetBoard (VOID)
{
  return 0;
}

/* ---- VS register hooks ------------------------------------------------- */
/*
 * Samsung's UfsDxe uses a vendor-specific (VS) register window that does
 * not exist on MTK's UFSHCI.  Writes are ignored; reads return 0 so loops
 * such as "wait for VS_SW_RST to clear" terminate immediately.
 */
VOID
MtkUfsVsWrite32 (
  struct UfsHost  *Ufs,
  UINTN            Offset,
  UINT32           Value
  )
{
  /* MT6893: no Samsung-style VS registers; SiP handles clock/reset. */
}

UINT32
MtkUfsVsRead32 (
  struct UfsHost  *Ufs,
  UINTN            Offset
  )
{
  return 0;
}

/* ---- Device power / PHY isolation hooks -------------------------------- */
VOID
MtkUfsDevPwrWrite (
  struct UfsHost  *Ufs,
  UINT32           Value
  )
{
  /* VCC is supplied by a fixed regulator; no runtime GPIO control. */
}

UINT32
MtkUfsDevPwrRead (struct UfsHost *Ufs)
{
  return 1;   /* device power always on */
}

VOID
MtkUfsPhyIsoWrite (
  struct UfsHost  *Ufs,
  UINT32           Value
  )
{
  /* PHY isolation is handled through SiP MTCMOS control. */
}

UINT32
MtkUfsPhyIsoRead (struct UfsHost *Ufs)
{
  return 1;   /* not isolated */
}
