/*++
	Copyright (c) Microsoft Corporation. All Rights Reserved.
	Sample code. Dealpoint ID #843729.

	Module Name:

		resolutions.h

	Abstract:

		Contains resolution translation defines and types

	Environment:

		Kernel mode

	Revision History:

--*/

#pragma once

#define TOUCH_SCREEN_PROPERTIES_REG_KEY L"\\Registry\\Machine\\System\\TOUCH\\SCREENPROPERTIES"
#define TOUCH_DEFAULT_RESOLUTION_X  480
#define TOUCH_DEFAULT_RESOLUTION_Y  800
#define TOUCH_DEVICE_RESOLUTION_X   1440
#define TOUCH_DEVICE_RESOLUTION_Y   2560

typedef struct _TOUCH_SCREEN_PROPERTIES
{
    UINT32 TouchSwapAxes;
    UINT32 TouchInvertXAxis;
    UINT32 TouchInvertYAxis;
    UINT32 TouchPhysicalWidth;
    UINT32 TouchPhysicalHeight;
    UINT32 TouchPhysicalButtonHeight;
    UINT32 TouchPillarBoxWidthLeft;
    UINT32 TouchPillarBoxWidthRight;
    UINT32 TouchLetterBoxHeightTop;
    UINT32 TouchLetterBoxHeightBottom;
    UINT32 DisplayPhysicalWidth;
    UINT32 DisplayPhysicalHeight;
    UINT32 DisplayViewableWidth;
    UINT32 DisplayViewableHeight;
    UINT32 DisplayPillarBoxWidthLeft;
    UINT32 DisplayPillarBoxWidthRight;
    UINT32 DisplayLetterBoxHeightTop;
    UINT32 DisplayLetterBoxHeightBottom;
    UINT32 DisplayHeight10um;
    UINT32 DisplayWidth10um;
    UINT32 TouchHardwareLacksContinuousReporting;
} TOUCH_SCREEN_PROPERTIES, * PTOUCH_SCREEN_PROPERTIES;

VOID
TchGetScreenProperties(
	IN PTOUCH_SCREEN_PROPERTIES Props
);

VOID
TchTranslateToDisplayCoordinates(
	IN PUSHORT X,
	IN PUSHORT Y,
	IN PTOUCH_SCREEN_PROPERTIES Props
);
