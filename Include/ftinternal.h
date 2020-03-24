/*++
	Copyright (c) Microsoft Corporation. All Rights Reserved.
	Sample code. Dealpoint ID #843729.

	Module Name:

		ftinternal.h

	Abstract:

		Contains common types and defintions used internally
		by the multi touch screen driver.

	Environment:

		Kernel mode

	Revision History:

--*/

#pragma once

#include <wdm.h>
#include <wdf.h>
#include "controller.h"
#include "resolutions.h"

typedef struct _FOCAL_TECH_TOUCH_DATA
{
	BYTE PositionX_High : 4;
	BYTE Reserved0 : 2;
	BYTE EventFlag : 2;

	BYTE PositionX_Low;

	BYTE PositionY_High : 4;
	BYTE TouchId : 4;

	BYTE PositionY_Low;

	BYTE Reserved1;

	BYTE Reserved2;
} FOCAL_TECH_TOUCH_DATA, * PFOCAL_TECH_TOUCH_DATA;

typedef struct _FOCAL_TECH_EVENT_DATA
{
	BYTE Reserved0 : 4;
	BYTE DeviceMode : 3;
	BYTE Reserved1 : 1;

	BYTE GestureId;

	BYTE NumberOfTouchPoints : 4;
	BYTE Reserved2 : 4;

	FOCAL_TECH_TOUCH_DATA TouchData[6];
} FOCAL_TECH_EVENT_DATA, * PFOCAL_TECH_EVENT_DATA;

#define TOUCH_POOL_TAG_F12              (ULONG)'21oT'

typedef struct _FT5X_CONTROLLER_CONTEXT
{
	WDFDEVICE FxDevice;
	WDFWAITLOCK ControllerLock;

	//
	// Power state
	//
	DEVICE_POWER_STATE DevicePowerState;

	//
	// Register configuration programmed to chip
	//
	TOUCH_SCREEN_PROPERTIES Props;

	UCHAR Data1Offset;
	BYTE MaxFingers;

    HID_INPUT_REPORT HidQueue[MAX_REPORTS_IN_QUEUE];
    int HidQueueCount;
} FT5X_CONTROLLER_CONTEXT;

NTSTATUS
GetNextHidReport(
    IN FT5X_CONTROLLER_CONTEXT* ControllerContext,
    IN PHID_INPUT_REPORT* HidReport
);
