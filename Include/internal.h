/*++
	Copyright (c) Microsoft Corporation. All Rights Reserved.
	Sample code. Dealpoint ID #843729.

	Module Name:

		internal.h

	Abstract:

		Contains common types and defintions used internally
		by the multi touch screen driver.

	Environment:

		Kernel mode

	Revision History:

--*/

#pragma once

#include "controller.h"

#define TOUCH_DELAY_TO_COMMUNICATE 200000
#define TOUCH_POWER_RAIL_STABLE_TIME 2000

//
// Device context
//

typedef struct _DEVICE_EXTENSION
{
	//
	// HID Touch input mode (touch vs. mouse)
	// 
	UCHAR InputMode;

	//
	// Device related
	//
	WDFDEVICE FxDevice;
	WDFQUEUE DefaultQueue;
	WDFQUEUE PingPongQueue;

	//
	// Interrupt servicing
	//
	WDFINTERRUPT InterruptObject;
	BOOLEAN ServiceInterruptsAfterD0Entry;

	//
	// Spb (I2C) related members used for the lifetime of the device
	//
	SPB_CONTEXT I2CContext;

	//
	// Reset GPIO line in case it exists used for power up sequence of the controller
	//
	LARGE_INTEGER ResetGpioId;
	WDFIOTARGET ResetGpio;
	BOOLEAN HasResetGpio;

	// 
	// Power related
	//
	WDFQUEUE IdleQueue;

	//
	// Touch related members used for the lifetime of the device
	//
	VOID* TouchContext;
} DEVICE_EXTENSION, * PDEVICE_EXTENSION;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_EXTENSION, GetDeviceContext)

void
SendHidReports(
    WDFQUEUE PingPongQueue,
    PHID_INPUT_REPORT hidReportsFromDriver,
    int hidReportsCount
);
