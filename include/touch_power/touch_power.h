/*++
    Copyright (c) Microsoft Corporation. All Rights Reserved.
    Copyright (c) LumiaWoA authors. All Rights Reserved.

    Module Name:

        power.h

    Abstract:

        Contains internal interfaces exposing functionality needed for
        controller self-tests, which can be used to run HW testing
        on a device in the manufacturing line, etc.

    Environment:

        Kernel mode

    Revision History:

--*/

#pragma once

#include <wdm.h>
#include <wdf.h>
#include <hidport.h>
#define RESHUB_USE_HELPER_ROUTINES
#include <reshub.h>
#include <initguid.h>

NTSTATUS
PowerInitialize(
    WDFDEVICE Device
);

NTSTATUS
PowerDeInitialize(
    WDFDEVICE Device
);

NTSTATUS
PowerToggle(
    TOUCH_POWER_CONTEXT* deviceContext,
    DWORD State
);

DRIVER_NOTIFICATION_CALLBACK_ROUTINE PowerIoRegPnPNotification;