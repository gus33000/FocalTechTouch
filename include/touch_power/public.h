/*++
    Copyright (c) Microsoft Corporation. All Rights Reserved.
    Copyright (c) LumiaWoA authors. All Rights Reserved.

    Module Name:

        public.h

    Abstract:

        Contains internal interfaces exposing functionality needed for
        controller power management.

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

//
// This GUID is used to access the touch self-test virtual device from user-mode
//
DEFINE_GUID(GUID_TOUCH_POWER_INTERFACE,
    0x9AE45E76, 0x6EF0, 0x4ED7, 0x85, 0xA2, 0x97, 0x71, 0x2A, 0x20, 0x78, 0x6A);
// {9AE45E76-6EF0-4ED7-85A2-97712A20786A}

#define TOUCH_POWER_BUFFER_CTL_CODE(id)  \
    CTL_CODE(0x8323, (id), METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_TOUCH_POWER_RESET           TOUCH_POWER_BUFFER_CTL_CODE(0x801)
#define IOCTL_TOUCH_POWER_TOGGLE          TOUCH_POWER_BUFFER_CTL_CODE(0x802)
#define IOCTL_TOUCH_POWER_STATE           TOUCH_POWER_BUFFER_CTL_CODE(0x803)