/*++
    Copyright (c) Microsoft Corporation. All Rights Reserved.
    Copyright (c) Bingxing Wang. All Rights Reserved.
    Copyright (c) LumiaWoA authors. All Rights Reserved.

    Module Name:

        registry.c

    Abstract:

        This module retrieves platform-specific controller
        configuration from the registry, or assigns default
        values if no registry configuration is present.

    Environment:

        Kernel mode

    Revision History:

--*/

#include <ft5x\ftinternal.h>
#include <registry.tmh>
#include <internal.h>

#define TOUCH_REG_KEY                    L"\\Registry\\Machine\\SYSTEM\\TOUCH"
#define TOUCH_SCREEN_SETTINGS_SUB_KEY    L"Settings"
#define TOUCH_SCREEN_SETTINGS_00_SUB_KEY L"Settings\\00"
#define TOUCH_SCREEN_SETTINGS_01_SUB_KEY L"Settings\\01"
#define TOUCH_SCREEN_SETTINGS_02_SUB_KEY L"Settings\\02"
#define TOUCH_SCREEN_SETTINGS_03_SUB_KEY L"Settings\\03"
#define TOUCH_SCREEN_SETTINGS_FF_SUB_KEY L"Settings\\FF"

//
// Default FT5X configuration values can be changed here. Please refer to the
// FT5X specification for a full description of the fields and value meanings
//

static FT5X_CONFIGURATION gDefaultConfiguration =
{
    //
    // FT5X F01 - Device control settings
    //
    {
        0,                                              // Sleep Mode (normal)
        1,                                              // No Sleep (do sleep)
        0,                                              // Report Rate (standard)
        1,                                              // Configured
        0xff,                                           // Interrupt Enable
        FT5X_MILLISECONDS_TO_TENTH_MILLISECONDS(20),    // Doze Interval
        10,                                             // Doze Threshold
        FT5X_SECONDS_TO_HALF_SECONDS(2)                 // Doze Holdoff
    },

    //
    // FT5X F11 - 2D Touchpad sensor settings
    //
    {
        1,                                              // Reporting mode (throttle)
        1,                                              // Abs position filter
        0,                                              // Rel position filter
        0,                                              // Rel ballistics
        0,                                              // Dribble
        0xb,                                            // PalmDetectThreshold
        3,                                              // MotionSensitivity
        0,                                              // ManTrackEn
        0,                                              // ManTrackedFinger
        0,                                              // DeltaXPosThreshold
        0,                                              // DeltaYPosThreshold
        0,                                              // Velocity
        0,                                              // Acceleration
        TOUCH_DEVICE_RESOLUTION_X,                      // Sensor Max X Position
        TOUCH_DEVICE_RESOLUTION_Y,                      // Sensor Max Y Position
        0x1e,                                           // ZTouchThreshold
        0x05,                                           // ZHysteresis
        0x28,                                           // SmallZThreshold
        0x28f5,                                         // SmallZScaleFactor
        0x051e,                                         // LargeZScaleFactor
        0x1,                                            // AlgorithmSelection
        0x30,                                           // WxScaleFactor
        0x0,                                            // WxOffset
        0x30,                                           // WyScaleFactor
        0x0,                                            // WyOffset
        0x4800,                                         // XPitch
        0x4800,                                         // YPitch
        0xea4f,                                         // FingerWidthX
        0xdf6c,                                         // FingerWidthY
        0,                                              // ReportMeasuredSize
        0x70,                                           // SegmentationSensitivity
        0x0,                                            // XClipLo
        0x0,                                            // XClipHi
        0x0,                                            // YClipLo
        0x0,                                            // YClipHi
        0x0a,                                           // MinFingerSeparation
        0x04                                            // MaxFingerMovement
    },

    //
    // Internal driver settings
    //
    {
        0x0,                                            // Controller stays powered in D3
    },
};

static TOUCH_SCREEN_SETTINGS gDefaultTouchSettings =
{
    0x1,
    0x0,
    0x1,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x3C,
    0x32,
    0x32,
    0x32,
    0x1,
    0x0,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x3FFF,
    0x3FFF,
    0x3FFF,
    0x0,
    0x3FFF,
    0x3FFF,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x3FFF,
    0x3FFF,
    0x3FFF,
    0x3FFF,
    0x3FFF,
    0x3FFF,
    0x0,
    0x0,
    0x0,
    0x3FFF,
    0x3FFF,
    0x0,
    0x0,
    0x3FFF,
    0x3FFF,
    0x3FFF,
    0x0,
    0x3FFF,
    0x3FFF,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x3FFF,
    0x3FFF,
    0x3FFF,
    0x3FFF,
    0x3FFF,
    0x3FFF,
    0x0,
    0x0,
    0x0,
    0x3FFF,
    0x3FFF,
    0x0,
    0x0,
    0x3FFF,
    0x3FFF,
    0x3FFF,
    0x0,
    0x3FFF,
    0x3FFF,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x3FFF,
    0x3FFF,
    0x3FFF,
    0x3FFF,
    0x3FFF,
    0x3FFF,
    0x0,
    0x0,
    0x0,
    0x3FFF,
    0x3FFF,
    0x0,
    0x0,
    0x3FFF,
    0x3FFF,
    0x3FFF,
    0x0,
    0x3FFF,
    0x3FFF,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x3FFF,
    0x3FFF,
    0x3FFF,
    0x3FFF,
    0x3FFF,
    0x3FFF,
    0x0,
    0x0,
    0x0,
    0x3FFF,
    0x3FFF,
    0x0,
};

RTL_QUERY_REGISTRY_TABLE gRegistryTable[] =
{
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"DeviceId",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, DeviceId)),
        REG_DWORD,
        &gDefaultTouchSettings.DeviceId,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"UseControllerSleep",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, UseControllerSleep)),
        REG_DWORD,
        &gDefaultTouchSettings.UseControllerSleep,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"UseNoSleepBit",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, UseNoSleepBit)),
        REG_DWORD,
        &gDefaultTouchSettings.UseNoSleepBit,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"ImprovedTouchSupported",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, ImprovedTouchSupported)),
        REG_DWORD,
        &gDefaultTouchSettings.ImprovedTouchSupported,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"WakeupGestureSupported",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, WakeupGestureSupported)),
        REG_DWORD,
        &gDefaultTouchSettings.WakeupGestureSupported,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"ChargerDetectionSupported",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, ChargerDetectionSupported)),
        REG_DWORD,
        &gDefaultTouchSettings.ChargerDetectionSupported,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"ActivePenSupported",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, ActivePenSupported)),
        REG_DWORD,
        &gDefaultTouchSettings.ActivePenSupported,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"ExtClockControlSupported",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, ExtClockControlSupported)),
        REG_DWORD,
        &gDefaultTouchSettings.ExtClockControlSupported,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"ForceDriverSupported",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, ForceDriverSupported)),
        REG_DWORD,
        &gDefaultTouchSettings.ForceDriverSupported,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"DoubleTapMaxTapTime10ms",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, DoubleTapMaxTapTime10ms)),
        REG_DWORD,
        &gDefaultTouchSettings.DoubleTapMaxTapTime10ms,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"DoubleTapMaxTapDistance100um",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, DoubleTapMaxTapDistance100um)),
        REG_DWORD,
        &gDefaultTouchSettings.DoubleTapMaxTapDistance100um,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"DoubleTapDeadZoneWidth100um",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, DoubleTapDeadZoneWidth100um)),
        REG_DWORD,
        &gDefaultTouchSettings.DoubleTapDeadZoneWidth100um,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"DoubleTapDeadZoneHeight100um",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, DoubleTapDeadZoneHeight100um)),
        REG_DWORD,
        &gDefaultTouchSettings.DoubleTapDeadZoneHeight100um,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"ControllerType",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, ControllerType)),
        REG_DWORD,
        &gDefaultTouchSettings.ControllerType,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"VendorCount",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, VendorCount)),
        REG_DWORD,
        &gDefaultTouchSettings.VendorCount,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"ResetControllerInWakeUp",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, ResetControllerInWakeUp)),
        REG_DWORD,
        &gDefaultTouchSettings.ResetControllerInWakeUp,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor00",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor00)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor00,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor01",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor01)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor01,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor02",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor02)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor02,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor03",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor03)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor03,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Revision00",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Revision00)),
        REG_DWORD,
        &gDefaultTouchSettings.Revision00,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Revision01",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Revision01)),
        REG_DWORD,
        &gDefaultTouchSettings.Revision01,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Revision02",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Revision02)),
        REG_DWORD,
        &gDefaultTouchSettings.Revision02,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Revision03",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Revision03)),
        REG_DWORD,
        &gDefaultTouchSettings.Revision03,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"ReprogramFw00",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, ReprogramFw00)),
        REG_DWORD,
        &gDefaultTouchSettings.ReprogramFw00,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"ReprogramFw01",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, ReprogramFw01)),
        REG_DWORD,
        &gDefaultTouchSettings.ReprogramFw01,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"ReprogramFw02",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, ReprogramFw02)),
        REG_DWORD,
        &gDefaultTouchSettings.ReprogramFw02,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"ReprogramFw03",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, ReprogramFw03)),
        REG_DWORD,
        &gDefaultTouchSettings.ReprogramFw03,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"ForceFlash",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, ForceFlash)),
        REG_DWORD,
        &gDefaultTouchSettings.ForceFlash,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor00ProductId0",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor00ProductId0)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor00ProductId0,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor00ProductId1",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor00ProductId1)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor00ProductId1,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor00ProductId2",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor00ProductId2)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor00ProductId2,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor00ProductId3",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor00ProductId3)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor00ProductId3,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor00ProductId4",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor00ProductId4)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor00ProductId4,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor00ProductId5",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor00ProductId5)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor00ProductId5,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor00ProductId6",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor00ProductId6)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor00ProductId6,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor00ProductId7",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor00ProductId7)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor00ProductId7,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor00ProductId8",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor00ProductId8)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor00ProductId8,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor00ProductId9",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor00ProductId9)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor00ProductId9,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor01ProductId0",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor01ProductId0)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor01ProductId0,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor01ProductId1",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor01ProductId1)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor01ProductId1,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor01ProductId2",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor01ProductId2)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor01ProductId2,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor01ProductId3",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor01ProductId3)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor01ProductId3,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor01ProductId4",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor01ProductId4)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor01ProductId4,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor01ProductId5",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor01ProductId5)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor01ProductId5,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor01ProductId6",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor01ProductId6)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor01ProductId6,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor01ProductId7",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor01ProductId7)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor01ProductId7,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor01ProductId8",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor01ProductId8)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor01ProductId8,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor01ProductId9",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor01ProductId9)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor01ProductId9,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor02ProductId0",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor02ProductId0)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor02ProductId0,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor02ProductId1",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor02ProductId1)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor02ProductId1,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor02ProductId2",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor02ProductId2)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor02ProductId2,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor02ProductId3",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor02ProductId3)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor02ProductId3,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor02ProductId4",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor02ProductId4)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor02ProductId4,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor02ProductId5",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor02ProductId5)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor02ProductId5,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor02ProductId6",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor02ProductId6)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor02ProductId6,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor02ProductId7",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor02ProductId7)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor02ProductId7,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor02ProductId8",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor02ProductId8)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor02ProductId8,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor02ProductId9",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor02ProductId9)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor02ProductId9,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor03ProductId0",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor03ProductId0)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor03ProductId0,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor03ProductId1",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor03ProductId1)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor03ProductId1,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor03ProductId2",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor03ProductId2)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor03ProductId2,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor03ProductId3",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor03ProductId3)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor03ProductId3,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor03ProductId4",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor03ProductId4)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor03ProductId4,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor03ProductId5",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor03ProductId5)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor03ProductId5,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor03ProductId6",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor03ProductId6)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor03ProductId6,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor03ProductId7",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor03ProductId7)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor03ProductId7,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor03ProductId8",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor03ProductId8)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor03ProductId8,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor03ProductId9",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor03ProductId9)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor03ProductId9,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor00IncludeHighResTest",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor00IncludeHighResTest)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor00IncludeHighResTest,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor00HighResMaxRxLimit",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor00HighResMaxRxLimit)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor00HighResMaxRxLimit,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor00HighResMaxTxLimit",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor00HighResMaxTxLimit)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor00HighResMaxTxLimit,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor00HighResMinImageLimit",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor00HighResMinImageLimit)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor00HighResMinImageLimit,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor00IncludeBaselineMinMaxTest",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor00IncludeBaselineMinMaxTest)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor00IncludeBaselineMinMaxTest,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor00BaselineMinMaxMinPixelLimit",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor00BaselineMinMaxMinPixelLimit)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor00BaselineMinMaxMinPixelLimit,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor00BaselineMinMaxMaxPixelLimit",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor00BaselineMinMaxMaxPixelLimit)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor00BaselineMinMaxMaxPixelLimit,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor00IncludeFullBaselineTest",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor00IncludeFullBaselineTest)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor00IncludeFullBaselineTest,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor00RxAmount",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor00RxAmount)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor00RxAmount,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor00TxAmount",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor00TxAmount)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor00TxAmount,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor00RxElectrodeMaskTouch2D",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor00RxElectrodeMaskTouch2D)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor00RxElectrodeMaskTouch2D,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor00TxElectrodeMaskTouch2D",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor00TxElectrodeMaskTouch2D)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor00TxElectrodeMaskTouch2D,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor00RxElectrodeMaskButtons",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor00RxElectrodeMaskButtons)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor00RxElectrodeMaskButtons,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor00TxElectrodeMaskButtons",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor00TxElectrodeMaskButtons)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor00TxElectrodeMaskButtons,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor00FullBaselineButton0Min",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor00FullBaselineButton0Min)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor00FullBaselineButton0Min,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor00FullBaselineButton1Min",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor00FullBaselineButton1Min)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor00FullBaselineButton1Min,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor00FullBaselineButton2Min",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor00FullBaselineButton2Min)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor00FullBaselineButton2Min,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor00FullBaselineButton0Max",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor00FullBaselineButton0Max)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor00FullBaselineButton0Max,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor00FullBaselineButton1Max",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor00FullBaselineButton1Max)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor00FullBaselineButton1Max,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor00FullBaselineButton2Max",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor00FullBaselineButton2Max)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor00FullBaselineButton2Max,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor00IncludeAbsSenseRawCapTest",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor00IncludeAbsSenseRawCapTest)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor00IncludeAbsSenseRawCapTest,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor00AbsSenseRawCapTxRxStart",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor00AbsSenseRawCapTxRxStart)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor00AbsSenseRawCapTxRxStart,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor00AbsSenseRawCapTxRxEnd",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor00AbsSenseRawCapTxRxEnd)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor00AbsSenseRawCapTxRxEnd,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor00AbsSenseRawCapMinLimit",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor00AbsSenseRawCapMinLimit)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor00AbsSenseRawCapMinLimit,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor00AbsSenseRawCapMaxLimit",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor00AbsSenseRawCapMaxLimit)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor00AbsSenseRawCapMaxLimit,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor00IncludeShortTest",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor00IncludeShortTest)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor00IncludeShortTest,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor01IncludeHighResTest",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor01IncludeHighResTest)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor01IncludeHighResTest,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor01HighResMaxRxLimit",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor01HighResMaxRxLimit)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor01HighResMaxRxLimit,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor01HighResMaxTxLimit",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor01HighResMaxTxLimit)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor01HighResMaxTxLimit,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor01HighResMinImageLimit",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor01HighResMinImageLimit)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor01HighResMinImageLimit,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor01IncludeBaselineMinMaxTest",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor01IncludeBaselineMinMaxTest)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor01IncludeBaselineMinMaxTest,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor01BaselineMinMaxMinPixelLimit",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor01BaselineMinMaxMinPixelLimit)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor01BaselineMinMaxMinPixelLimit,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor01BaselineMinMaxMaxPixelLimit",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor01BaselineMinMaxMaxPixelLimit)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor01BaselineMinMaxMaxPixelLimit,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor01IncludeFullBaselineTest",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor01IncludeFullBaselineTest)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor01IncludeFullBaselineTest,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor01RxAmount",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor01RxAmount)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor01RxAmount,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor01TxAmount",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor01TxAmount)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor01TxAmount,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor01RxElectrodeMaskTouch2D",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor01RxElectrodeMaskTouch2D)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor01RxElectrodeMaskTouch2D,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor01TxElectrodeMaskTouch2D",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor01TxElectrodeMaskTouch2D)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor01TxElectrodeMaskTouch2D,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor01RxElectrodeMaskButtons",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor01RxElectrodeMaskButtons)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor01RxElectrodeMaskButtons,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor01TxElectrodeMaskButtons",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor01TxElectrodeMaskButtons)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor01TxElectrodeMaskButtons,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor01FullBaselineButton0Min",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor01FullBaselineButton0Min)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor01FullBaselineButton0Min,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor01FullBaselineButton1Min",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor01FullBaselineButton1Min)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor01FullBaselineButton1Min,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor01FullBaselineButton2Min",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor01FullBaselineButton2Min)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor01FullBaselineButton2Min,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor01FullBaselineButton0Max",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor01FullBaselineButton0Max)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor01FullBaselineButton0Max,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor01FullBaselineButton1Max",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor01FullBaselineButton1Max)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor01FullBaselineButton1Max,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor01FullBaselineButton2Max",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor01FullBaselineButton2Max)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor01FullBaselineButton2Max,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor01IncludeAbsSenseRawCapTest",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor01IncludeAbsSenseRawCapTest)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor01IncludeAbsSenseRawCapTest,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor01AbsSenseRawCapTxRxStart",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor01AbsSenseRawCapTxRxStart)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor01AbsSenseRawCapTxRxStart,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor01AbsSenseRawCapTxRxEnd",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor01AbsSenseRawCapTxRxEnd)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor01AbsSenseRawCapTxRxEnd,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor01AbsSenseRawCapMinLimit",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor01AbsSenseRawCapMinLimit)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor01AbsSenseRawCapMinLimit,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor01AbsSenseRawCapMaxLimit",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor01AbsSenseRawCapMaxLimit)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor01AbsSenseRawCapMaxLimit,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor01IncludeShortTest",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor01IncludeShortTest)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor01IncludeShortTest,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor02IncludeHighResTest",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor02IncludeHighResTest)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor02IncludeHighResTest,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor02HighResMaxRxLimit",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor02HighResMaxRxLimit)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor02HighResMaxRxLimit,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor02HighResMaxTxLimit",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor02HighResMaxTxLimit)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor02HighResMaxTxLimit,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor02HighResMinImageLimit",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor02HighResMinImageLimit)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor02HighResMinImageLimit,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor02IncludeBaselineMinMaxTest",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor02IncludeBaselineMinMaxTest)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor02IncludeBaselineMinMaxTest,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor02BaselineMinMaxMinPixelLimit",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor02BaselineMinMaxMinPixelLimit)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor02BaselineMinMaxMinPixelLimit,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor02BaselineMinMaxMaxPixelLimit",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor02BaselineMinMaxMaxPixelLimit)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor02BaselineMinMaxMaxPixelLimit,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor02IncludeFullBaselineTest",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor02IncludeFullBaselineTest)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor02IncludeFullBaselineTest,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor02RxAmount",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor02RxAmount)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor02RxAmount,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor02TxAmount",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor02TxAmount)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor02TxAmount,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor02RxElectrodeMaskTouch2D",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor02RxElectrodeMaskTouch2D)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor02RxElectrodeMaskTouch2D,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor02TxElectrodeMaskTouch2D",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor02TxElectrodeMaskTouch2D)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor02TxElectrodeMaskTouch2D,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor02RxElectrodeMaskButtons",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor02RxElectrodeMaskButtons)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor02RxElectrodeMaskButtons,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor02TxElectrodeMaskButtons",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor02TxElectrodeMaskButtons)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor02TxElectrodeMaskButtons,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor02FullBaselineButton0Min",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor02FullBaselineButton0Min)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor02FullBaselineButton0Min,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor02FullBaselineButton1Min",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor02FullBaselineButton1Min)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor02FullBaselineButton1Min,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor02FullBaselineButton2Min",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor02FullBaselineButton2Min)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor02FullBaselineButton2Min,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor02FullBaselineButton0Max",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor02FullBaselineButton0Max)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor02FullBaselineButton0Max,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor02FullBaselineButton1Max",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor02FullBaselineButton1Max)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor02FullBaselineButton1Max,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor02FullBaselineButton2Max",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor02FullBaselineButton2Max)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor02FullBaselineButton2Max,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor02IncludeAbsSenseRawCapTest",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor02IncludeAbsSenseRawCapTest)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor02IncludeAbsSenseRawCapTest,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor02AbsSenseRawCapTxRxStart",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor02AbsSenseRawCapTxRxStart)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor02AbsSenseRawCapTxRxStart,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor02AbsSenseRawCapTxRxEnd",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor02AbsSenseRawCapTxRxEnd)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor02AbsSenseRawCapTxRxEnd,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor02AbsSenseRawCapMinLimit",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor02AbsSenseRawCapMinLimit)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor02AbsSenseRawCapMinLimit,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor02AbsSenseRawCapMaxLimit",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor02AbsSenseRawCapMaxLimit)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor02AbsSenseRawCapMaxLimit,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor02IncludeShortTest",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor02IncludeShortTest)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor02IncludeShortTest,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor03IncludeHighResTest",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor03IncludeHighResTest)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor03IncludeHighResTest,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor03HighResMaxRxLimit",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor03HighResMaxRxLimit)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor03HighResMaxRxLimit,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor03HighResMaxTxLimit",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor03HighResMaxTxLimit)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor03HighResMaxTxLimit,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor03HighResMinImageLimit",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor03HighResMinImageLimit)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor03HighResMinImageLimit,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor03IncludeBaselineMinMaxTest",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor03IncludeBaselineMinMaxTest)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor03IncludeBaselineMinMaxTest,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor03BaselineMinMaxMinPixelLimit",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor03BaselineMinMaxMinPixelLimit)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor03BaselineMinMaxMinPixelLimit,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor03BaselineMinMaxMaxPixelLimit",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor03BaselineMinMaxMaxPixelLimit)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor03BaselineMinMaxMaxPixelLimit,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor03IncludeFullBaselineTest",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor03IncludeFullBaselineTest)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor03IncludeFullBaselineTest,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor03RxAmount",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor03RxAmount)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor03RxAmount,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor03TxAmount",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor03TxAmount)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor03TxAmount,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor03RxElectrodeMaskTouch2D",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor03RxElectrodeMaskTouch2D)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor03RxElectrodeMaskTouch2D,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor03TxElectrodeMaskTouch2D",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor03TxElectrodeMaskTouch2D)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor03TxElectrodeMaskTouch2D,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor03RxElectrodeMaskButtons",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor03RxElectrodeMaskButtons)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor03RxElectrodeMaskButtons,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor03TxElectrodeMaskButtons",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor03TxElectrodeMaskButtons)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor03TxElectrodeMaskButtons,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor03FullBaselineButton0Min",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor03FullBaselineButton0Min)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor03FullBaselineButton0Min,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor03FullBaselineButton1Min",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor03FullBaselineButton1Min)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor03FullBaselineButton1Min,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor03FullBaselineButton2Min",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor03FullBaselineButton2Min)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor03FullBaselineButton2Min,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor03FullBaselineButton0Max",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor03FullBaselineButton0Max)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor03FullBaselineButton0Max,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor03FullBaselineButton1Max",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor03FullBaselineButton1Max)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor03FullBaselineButton1Max,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor03FullBaselineButton2Max",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor03FullBaselineButton2Max)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor03FullBaselineButton2Max,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor03IncludeAbsSenseRawCapTest",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor03IncludeAbsSenseRawCapTest)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor03IncludeAbsSenseRawCapTest,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor03AbsSenseRawCapTxRxStart",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor03AbsSenseRawCapTxRxStart)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor03AbsSenseRawCapTxRxStart,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor03AbsSenseRawCapTxRxEnd",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor03AbsSenseRawCapTxRxEnd)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor03AbsSenseRawCapTxRxEnd,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor03AbsSenseRawCapMinLimit",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor03AbsSenseRawCapMinLimit)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor03AbsSenseRawCapMinLimit,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor03AbsSenseRawCapMaxLimit",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor03AbsSenseRawCapMaxLimit)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor03AbsSenseRawCapMaxLimit,
        sizeof(UINT32)
    },
    {
        NULL, RTL_QUERY_REGISTRY_DIRECT,
        L"Vendor03IncludeShortTest",
        (PVOID)(FIELD_OFFSET(TOUCH_SCREEN_SETTINGS, Vendor03IncludeShortTest)),
        REG_DWORD,
        &gDefaultTouchSettings.Vendor03IncludeShortTest,
        sizeof(UINT32)
    },
    //
    // List Terminator
    //
    {
        NULL, 0,
        NULL,
        0,
        REG_DWORD,
        NULL,
        0
    }
};
static const ULONG gcbRegistryTable = sizeof(gRegistryTable);
static const ULONG gcRegistryTable =
sizeof(gRegistryTable) / sizeof(gRegistryTable[0]);

NTSTATUS
RtlReadRegistryValue(
    PCWSTR registry_path,
    PCWSTR value_name,
    ULONG type,
    PVOID data,
    ULONG length
)
{
    UNICODE_STRING valname;
    UNICODE_STRING keyname;
    OBJECT_ATTRIBUTES attribs;
    PKEY_VALUE_PARTIAL_INFORMATION pinfo;
    HANDLE handle;
    NTSTATUS rc;
    ULONG len, reslen;

    RtlInitUnicodeString(&keyname, registry_path);
    RtlInitUnicodeString(&valname, value_name);

    InitializeObjectAttributes(
        &attribs, 
        &keyname, 
        OBJ_CASE_INSENSITIVE,
        NULL, 
        NULL
    );

    rc = ZwOpenKey(
        &handle, 
        KEY_QUERY_VALUE, 
        &attribs
    );

    if (!NT_SUCCESS(rc))
    {
        return 0;
    }

    len = sizeof(KEY_VALUE_PARTIAL_INFORMATION) + length;

    pinfo = ExAllocatePoolWithTag(
        NonPagedPool, 
        len, 
        TOUCH_POOL_TAG
    );

    if (pinfo == NULL)
    {
        goto exit;
    }

    rc = ZwQueryValueKey(
        handle, 
        &valname, 
        KeyValuePartialInformation,
        pinfo, 
        len, 
        &reslen
    );

    if ((NT_SUCCESS(rc) || rc == STATUS_BUFFER_OVERFLOW) && 
        reslen >= (sizeof(KEY_VALUE_PARTIAL_INFORMATION) - 1) &&
        (!type || pinfo->Type == type))
    {
        reslen = pinfo->DataLength;
        memcpy(data, pinfo->Data, min(length, reslen));
    }
    else
    {
        reslen = 0;
    }

    if (pinfo != NULL)
    {
        ExFreePoolWithTag(pinfo, TOUCH_POOL_TAG);
    }

exit:
    ZwClose(handle);
    return rc;
}

NTSTATUS
TchRegistryGetControllerSettings(
    IN VOID* ControllerContext,
    IN WDFDEVICE FxDevice
)
/*++

  Routine Description:

    This routine retrieves controller wide settings
    from the registry.

  Arguments:

    FxDevice - a handle to the framework device object
    Settings - A pointer to the chip settings structure

  Return Value:

    NTSTATUS indicating success or failure

--*/
{
    FT5X_CONTROLLER_CONTEXT* controller;
    NTSTATUS status;

    UNREFERENCED_PARAMETER(FxDevice);

    controller = (FT5X_CONTROLLER_CONTEXT*)ControllerContext;

    RtlCopyMemory(
        &controller->Config,
        &gDefaultConfiguration,
        sizeof(FT5X_CONFIGURATION));

    status = STATUS_SUCCESS;

    return status;
}

/*
 * Appends src to string dst of size siz (unlike strncat, siz is the
 * full size of dst, not space left).  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz = siz, truncation occurred.
 * 
 * From: https://stackoverflow.com/questions/1855956/how-do-you-concatenate-two-wchar-t-together
 */
size_t wstrlcat(wchar_t* dst, const wchar_t* src, size_t siz)
{
    wchar_t* d = dst;
    const wchar_t* s = src;
    size_t n = siz;
    size_t dlen;

    /* Find the end of dst and adjust bytes left but don't go past end */
    while (n-- != 0 && *d != L'\0') {
        d++;
    }

    dlen = d - dst;
    n = siz - dlen;

    if (n == 0) {
        return(dlen + wcslen(s));
    }

    while (*s != L'\0')
    {
        if (n != 1)
        {
            *d++ = *s;
            n--;
        }
        s++;
    }

    *d = '\0';
    return(dlen + (s - src));        /* count does not include NUL */
}

VOID
TchGetTouchSettings(
    IN PTOUCH_SCREEN_SETTINGS TouchSettings
)
{
    ULONG i;
    PRTL_QUERY_REGISTRY_TABLE regTable;
    WCHAR regKey[120] = { 0 };
    NTSTATUS status;

    regTable = NULL;

    wstrlcat(regKey, TOUCH_REG_KEY, sizeof(TOUCH_REG_KEY));
    regKey[sizeof(TOUCH_REG_KEY) / sizeof(WCHAR)] = L'\\';
    RtlCopyMemory((PCHAR)regKey + sizeof(TOUCH_REG_KEY) + sizeof(WCHAR), TOUCH_SCREEN_SETTINGS_SUB_KEY, sizeof(TOUCH_SCREEN_SETTINGS_SUB_KEY) - sizeof(WCHAR));
    regKey[(sizeof(TOUCH_REG_KEY) + sizeof(TOUCH_SCREEN_SETTINGS_SUB_KEY)) / sizeof(WCHAR)] = L'\0';

    //
    // Table passed to RtlQueryRegistryValues must be allocated 
    // from NonPagedPool
    //
    regTable = ExAllocatePoolWithTag(
        NonPagedPool,
        gcbRegistryTable,
        TOUCH_POOL_TAG);

    if (regTable == NULL)
    {
        return;
    }

    RtlCopyMemory(
        regTable,
        gRegistryTable,
        gcbRegistryTable);

    //
    // Update offset values with base pointer
    // 
    for (i = 0; i < gcRegistryTable - 1; i++)
    {
        regTable[i].EntryContext = (PVOID)(
            ((SIZE_T)regTable[i].EntryContext) +
            ((ULONG_PTR)TouchSettings));
    }

    //
    // Start with default values
    //
    RtlCopyMemory(
        TouchSettings,
        &gDefaultTouchSettings,
        sizeof(TOUCH_SCREEN_SETTINGS));

    //
    // Populate device context with registry overrides (or defaults)
    //
    status = RtlQueryRegistryValues(
        RTL_REGISTRY_ABSOLUTE,
        regKey,
        regTable,
        NULL,
        NULL);

    if (!NT_SUCCESS(status))
    {
        Trace(
            TRACE_LEVEL_WARNING,
            TRACE_REGISTRY,
            "Error retrieving registry configuration - 0x%08lX",
            status);
    }

    if (regTable != NULL)
    {
        ExFreePoolWithTag(regTable, TOUCH_POOL_TAG);
    }
}