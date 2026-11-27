#ifndef __PLATFORM_UFS_LIB_H__
#define __PLATFORM_UFS_LIB_H__

#include <Uefi.h>
#include <Library/UfsHostBridge.h>
#include <Library/UfsCalAdapterLib.h>

/*
 * MT6893 (Dimensity 1200) UFS platform hooks.
 *
 * The Samsung UfsDxe core is reused because the UFSHCI / UTP / UIC / query
 * / BlockIo layers are JEDEC-standard.  All vendor-specific accesses are
 * routed here so that MTK's different register semantics are isolated.
 */

EFI_STATUS UfsBoardInit (struct UfsHost *Ufs);

/* Calibration hooks (no-op for MT6893: PHY is set up by UfsBoardInit/ATF). */
UfsCalError UfsCalInit    (struct UfsCalParam *p);
UfsCalError UfsCalPreLink (struct UfsCalParam *p);
UfsCalError UfsCalPostLink (struct UfsCalParam *p);
UfsCalError UfsCalPrePmc  (struct UfsCalParam *p);
UfsCalError UfsCalPostPmc (struct UfsCalParam *p);
UINT8       UfsCalGetTargetBoard (VOID);

/* VS-register access hooks (MTK UFSHCI vendor region). */
VOID   MtkUfsVsWrite32  (struct UfsHost *Ufs, UINTN Offset, UINT32 Value);
UINT32 MtkUfsVsRead32   (struct UfsHost *Ufs, UINTN Offset);

/* Device power / PHY isolation hooks. */
VOID   MtkUfsDevPwrWrite (struct UfsHost *Ufs, UINT32 Value);
UINT32 MtkUfsDevPwrRead  (struct UfsHost *Ufs);
VOID   MtkUfsPhyIsoWrite (struct UfsHost *Ufs, UINT32 Value);
UINT32 MtkUfsPhyIsoRead  (struct UfsHost *Ufs);

#endif /* __PLATFORM_UFS_LIB_H__ */
