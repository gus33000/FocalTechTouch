/*++
	Copyright (c) Microsoft Corporation. All Rights Reserved.
	Copyright (c) Bingxing Wang. All Rights Reserved.
	Copyright (c) LumiaWoA authors. All Rights Reserved.

	Module Name:

		report.c

	Abstract:

		Contains FocalTech specific code for reporting samples

	Environment:

		Kernel mode

	Revision History:

--*/

#pragma once

#include <Cross Platform Shim\compat.h>
#include <controller.h>
#include <resolutions.h>
#include <hid.h>
#include <HidCommon.h>
#include <spb.h>

#define MAX_TOUCHES                32
#define MAX_BUTTONS                3

typedef struct _OBJECT_INFO
{
	int x;
	int y;
	UCHAR status;
} OBJECT_INFO;

typedef struct _OBJECT_CACHE
{
	OBJECT_INFO Slot[MAX_TOUCHES];
	UINT32 SlotValid;
	UINT32 SlotDirty;
	int DownOrder[MAX_TOUCHES];
	int DownCount;
	ULONG64 ScanTime;
} OBJECT_CACHE;

typedef struct _DETECTED_OBJECT_POSITION
{
	int X;
	int Y;
} DETECTED_OBJECT_POSITION;

typedef enum _OBJECT_STATE
{
	OBJECT_STATE_NOT_PRESENT = 0,
	OBJECT_STATE_FINGER_PRESENT_WITH_ACCURATE_POS = 1,
	OBJECT_STATE_FINGER_PRESENT_WITH_INACCURATE_POS = 2,
	OBJECT_STATE_PEN_PRESENT_WITH_TIP = 3,
	OBJECT_STATE_PEN_PRESENT_WITH_ERASER = 4,
	OBJECT_STATE_RESERVED = 5
} OBJECT_STATE;

typedef struct _DETECTED_OBJECTS
{
	OBJECT_STATE States[MAX_TOUCHES];
	DETECTED_OBJECT_POSITION Positions[MAX_TOUCHES];
} DETECTED_OBJECTS;

typedef struct _BUTTON_CACHE
{
	BOOLEAN ButtonSlots[MAX_BUTTONS];
} BUTTON_CACHE;

typedef struct _REPORT_CONTEXT
{
	BUTTON_CACHE ButtonCache;
	BOOLEAN PenPresent;
	OBJECT_CACHE Cache;
	TOUCH_SCREEN_PROPERTIES Props;
	WDFQUEUE PingPongQueue;
} REPORT_CONTEXT, * PREPORT_CONTEXT;

NTSTATUS
ReportWakeup(
	IN PREPORT_CONTEXT ReportContext
);

NTSTATUS
ReportKeypad(
	IN PREPORT_CONTEXT ReportContext,
	IN BOOLEAN Back,
	IN BOOLEAN Start,
	IN BOOLEAN Search
);

NTSTATUS
ReportPen(
	IN PREPORT_CONTEXT ReportContext,
	IN BOOLEAN TipSwitch,
	IN BOOLEAN BarrelSwitch,
	IN BOOLEAN Invert,
	IN BOOLEAN Eraser,
	IN BOOLEAN InRange,
	IN USHORT X,
	IN USHORT Y,
	IN USHORT TipPressure,
	IN USHORT XTilt,
	IN USHORT YTilt
);

NTSTATUS
ReportObjects(
	IN PREPORT_CONTEXT ReportContext,
	IN DETECTED_OBJECTS data
);

NTSTATUS
ReportConfigureContinuousSimulationTimer(
	IN WDFDEVICE DeviceHandle
);