// Copyright (c) Microsoft Corporation. All Rights Reserved. 
// Copyright (c) Bingxing Wang. All Rights Reserved. 

#pragma once

#include <wdm.h>
#include <wdf.h>
#include <hidport.h>
#define RESHUB_USE_HELPER_ROUTINES
#include <reshub.h>
#include "trace.h"
#include "hid.h"
#include "spb.h"

//
// Memory tags
//
#define TOUCH_POOL_TAG                  (ULONG)'cuoT'
#define TOUCH_POOL_TAG_F12              (ULONG)'21oT'
#define TOUCH_POWER_POOL_TAG            (ULONG)'PuoT'

//
// Constants
//
#define MODE_MULTI_TOUCH                0x02
#define MAX_TOUCH_COORD                 0x0FFF
#define FINGER_STATUS                   0x01 // finger down

//
// Structures
//
typedef struct _TOUCH_SCREEN_SETTINGS
{
	UINT32 DeviceId;
	UINT32 UseControllerSleep;
	UINT32 UseNoSleepBit;
	UINT32 ImprovedTouchSupported;
	UINT32 WakeupGestureSupported;
	UINT32 ChargerDetectionSupported;
	UINT32 ActivePenSupported;
	UINT32 ExtClockControlSupported;
	UINT32 ForceDriverSupported;
	UINT32 DoubleTapMaxTapTime10ms;
	UINT32 DoubleTapMaxTapDistance100um;
	UINT32 DoubleTapDeadZoneWidth100um;
	UINT32 DoubleTapDeadZoneHeight100um;
	UINT32 ControllerType;
	UINT32 VendorCount;
	UINT32 ResetControllerInWakeUp;
	UINT32 Vendor00;
	UINT32 Vendor01;
	UINT32 Vendor02;
	UINT32 Vendor03;
	UINT32 Revision00;
	UINT32 Revision01;
	UINT32 Revision02;
	UINT32 Revision03;
	UINT32 ReprogramFw00;
	UINT32 ReprogramFw01;
	UINT32 ReprogramFw02;
	UINT32 ReprogramFw03;
	UINT32 ForceFlash;
	UINT32 Vendor00ProductId0;
	UINT32 Vendor00ProductId1;
	UINT32 Vendor00ProductId2;
	UINT32 Vendor00ProductId3;
	UINT32 Vendor00ProductId4;
	UINT32 Vendor00ProductId5;
	UINT32 Vendor00ProductId6;
	UINT32 Vendor00ProductId7;
	UINT32 Vendor00ProductId8;
	UINT32 Vendor00ProductId9;
	UINT32 Vendor01ProductId0;
	UINT32 Vendor01ProductId1;
	UINT32 Vendor01ProductId2;
	UINT32 Vendor01ProductId3;
	UINT32 Vendor01ProductId4;
	UINT32 Vendor01ProductId5;
	UINT32 Vendor01ProductId6;
	UINT32 Vendor01ProductId7;
	UINT32 Vendor01ProductId8;
	UINT32 Vendor01ProductId9;
	UINT32 Vendor02ProductId0;
	UINT32 Vendor02ProductId1;
	UINT32 Vendor02ProductId2;
	UINT32 Vendor02ProductId3;
	UINT32 Vendor02ProductId4;
	UINT32 Vendor02ProductId5;
	UINT32 Vendor02ProductId6;
	UINT32 Vendor02ProductId7;
	UINT32 Vendor02ProductId8;
	UINT32 Vendor02ProductId9;
	UINT32 Vendor03ProductId0;
	UINT32 Vendor03ProductId1;
	UINT32 Vendor03ProductId2;
	UINT32 Vendor03ProductId3;
	UINT32 Vendor03ProductId4;
	UINT32 Vendor03ProductId5;
	UINT32 Vendor03ProductId6;
	UINT32 Vendor03ProductId7;
	UINT32 Vendor03ProductId8;
	UINT32 Vendor03ProductId9;
	UINT32 Vendor00IncludeHighResTest;
	UINT32 Vendor00HighResMaxRxLimit;
	UINT32 Vendor00HighResMaxTxLimit;
	UINT32 Vendor00HighResMinImageLimit;
	UINT32 Vendor00IncludeBaselineMinMaxTest;
	UINT32 Vendor00BaselineMinMaxMinPixelLimit;
	UINT32 Vendor00BaselineMinMaxMaxPixelLimit;
	UINT32 Vendor00IncludeFullBaselineTest;
	UINT32 Vendor00RxAmount;
	UINT32 Vendor00TxAmount;
	UINT32 Vendor00RxElectrodeMaskTouch2D;
	UINT32 Vendor00TxElectrodeMaskTouch2D;
	UINT32 Vendor00RxElectrodeMaskButtons;
	UINT32 Vendor00TxElectrodeMaskButtons;
	UINT32 Vendor00FullBaselineButton0Min;
	UINT32 Vendor00FullBaselineButton1Min;
	UINT32 Vendor00FullBaselineButton2Min;
	UINT32 Vendor00FullBaselineButton0Max;
	UINT32 Vendor00FullBaselineButton1Max;
	UINT32 Vendor00FullBaselineButton2Max;
	UINT32 Vendor00IncludeAbsSenseRawCapTest;
	UINT32 Vendor00AbsSenseRawCapTxRxStart;
	UINT32 Vendor00AbsSenseRawCapTxRxEnd;
	UINT32 Vendor00AbsSenseRawCapMinLimit;
	UINT32 Vendor00AbsSenseRawCapMaxLimit;
	UINT32 Vendor00IncludeShortTest;
	UINT32 Vendor01IncludeHighResTest;
	UINT32 Vendor01HighResMaxRxLimit;
	UINT32 Vendor01HighResMaxTxLimit;
	UINT32 Vendor01HighResMinImageLimit;
	UINT32 Vendor01IncludeBaselineMinMaxTest;
	UINT32 Vendor01BaselineMinMaxMinPixelLimit;
	UINT32 Vendor01BaselineMinMaxMaxPixelLimit;
	UINT32 Vendor01IncludeFullBaselineTest;
	UINT32 Vendor01RxAmount;
	UINT32 Vendor01TxAmount;
	UINT32 Vendor01RxElectrodeMaskTouch2D;
	UINT32 Vendor01TxElectrodeMaskTouch2D;
	UINT32 Vendor01RxElectrodeMaskButtons;
	UINT32 Vendor01TxElectrodeMaskButtons;
	UINT32 Vendor01FullBaselineButton0Min;
	UINT32 Vendor01FullBaselineButton1Min;
	UINT32 Vendor01FullBaselineButton2Min;
	UINT32 Vendor01FullBaselineButton0Max;
	UINT32 Vendor01FullBaselineButton1Max;
	UINT32 Vendor01FullBaselineButton2Max;
	UINT32 Vendor01IncludeAbsSenseRawCapTest;
	UINT32 Vendor01AbsSenseRawCapTxRxStart;
	UINT32 Vendor01AbsSenseRawCapTxRxEnd;
	UINT32 Vendor01AbsSenseRawCapMinLimit;
	UINT32 Vendor01AbsSenseRawCapMaxLimit;
	UINT32 Vendor01IncludeShortTest;
	UINT32 Vendor02IncludeHighResTest;
	UINT32 Vendor02HighResMaxRxLimit;
	UINT32 Vendor02HighResMaxTxLimit;
	UINT32 Vendor02HighResMinImageLimit;
	UINT32 Vendor02IncludeBaselineMinMaxTest;
	UINT32 Vendor02BaselineMinMaxMinPixelLimit;
	UINT32 Vendor02BaselineMinMaxMaxPixelLimit;
	UINT32 Vendor02IncludeFullBaselineTest;
	UINT32 Vendor02RxAmount;
	UINT32 Vendor02TxAmount;
	UINT32 Vendor02RxElectrodeMaskTouch2D;
	UINT32 Vendor02TxElectrodeMaskTouch2D;
	UINT32 Vendor02RxElectrodeMaskButtons;
	UINT32 Vendor02TxElectrodeMaskButtons;
	UINT32 Vendor02FullBaselineButton0Min;
	UINT32 Vendor02FullBaselineButton1Min;
	UINT32 Vendor02FullBaselineButton2Min;
	UINT32 Vendor02FullBaselineButton0Max;
	UINT32 Vendor02FullBaselineButton1Max;
	UINT32 Vendor02FullBaselineButton2Max;
	UINT32 Vendor02IncludeAbsSenseRawCapTest;
	UINT32 Vendor02AbsSenseRawCapTxRxStart;
	UINT32 Vendor02AbsSenseRawCapTxRxEnd;
	UINT32 Vendor02AbsSenseRawCapMinLimit;
	UINT32 Vendor02AbsSenseRawCapMaxLimit;
	UINT32 Vendor02IncludeShortTest;
	UINT32 Vendor03IncludeHighResTest;
	UINT32 Vendor03HighResMaxRxLimit;
	UINT32 Vendor03HighResMaxTxLimit;
	UINT32 Vendor03HighResMinImageLimit;
	UINT32 Vendor03IncludeBaselineMinMaxTest;
	UINT32 Vendor03BaselineMinMaxMinPixelLimit;
	UINT32 Vendor03BaselineMinMaxMaxPixelLimit;
	UINT32 Vendor03IncludeFullBaselineTest;
	UINT32 Vendor03RxAmount;
	UINT32 Vendor03TxAmount;
	UINT32 Vendor03RxElectrodeMaskTouch2D;
	UINT32 Vendor03TxElectrodeMaskTouch2D;
	UINT32 Vendor03RxElectrodeMaskButtons;
	UINT32 Vendor03TxElectrodeMaskButtons;
	UINT32 Vendor03FullBaselineButton0Min;
	UINT32 Vendor03FullBaselineButton1Min;
	UINT32 Vendor03FullBaselineButton2Min;
	UINT32 Vendor03FullBaselineButton0Max;
	UINT32 Vendor03FullBaselineButton1Max;
	UINT32 Vendor03FullBaselineButton2Max;
	UINT32 Vendor03IncludeAbsSenseRawCapTest;
	UINT32 Vendor03AbsSenseRawCapTxRxStart;
	UINT32 Vendor03AbsSenseRawCapTxRxEnd;
	UINT32 Vendor03AbsSenseRawCapMinLimit;
	UINT32 Vendor03AbsSenseRawCapMaxLimit;
	UINT32 Vendor03IncludeShortTest;
} TOUCH_SCREEN_SETTINGS, * PTOUCH_SCREEN_SETTINGS;

NTSTATUS 
TchAllocateContext(
    OUT VOID **ControllerContext,
    IN WDFDEVICE FxDevice
    );

NTSTATUS 
TchFreeContext(
    IN VOID *ControllerContext
    );

NTSTATUS
TchStartDevice(
	IN VOID* ControllerContext,
	IN SPB_CONTEXT* SpbContext
);

NTSTATUS 
TchStopDevice(
    IN VOID *ControllerContext,
    IN SPB_CONTEXT *SpbContext
    );

NTSTATUS
TchStandbyDevice(
	IN VOID* ControllerContext,
	IN SPB_CONTEXT* SpbContext,
	IN VOID* ReportContext
	);

NTSTATUS 
TchWakeDevice(
    IN VOID *ControllerContext,
    IN SPB_CONTEXT *SpbContext
    );

NTSTATUS
RtlReadRegistryValue(
    PCWSTR registry_path, 
    PCWSTR value_name, 
    ULONG type, 
    PVOID data, 
    ULONG length
    );

NTSTATUS
TchRegistryGetControllerSettings(
    IN VOID *ControllerContext,
    IN WDFDEVICE FxDevice
    );

VOID
TchGetTouchSettings(
	IN PTOUCH_SCREEN_SETTINGS TouchSettings
);

NTSTATUS
TchPowerSettingCallback(
    _In_ LPCGUID SettingGuid,
    _In_ PVOID Value,
    _In_ ULONG ValueLength,
    _Inout_opt_ PVOID Context
);