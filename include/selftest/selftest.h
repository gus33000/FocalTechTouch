/*++
    Copyright (c) Microsoft Corporation. All Rights Reserved. 
    Sample code. Dealpoint ID #843729.

    Module Name:

        selftest.h

    Abstract:

        Contains internal interfaces exposing functionality needed for 
        controller self-tests, which can be used to run HW testing 
        on a device in the manufacturing line, etc.

    Environment:

        Kernel mode

    Revision History:

--*/

#pragma once


//
// This GUID is used to access the touch self-test virtual device from user-mode
//
DEFINE_GUID(GUID_TOUCH_SELFTEST_INTERFACE, 
   0x3a0ac59a, 0x4d8a, 0x4875, 0xb7, 0xea, 0x30, 0x47, 0x71, 0xff, 0x9b, 0x9a);
// {3a0ac59a-4d8a-4875-b7ea-304771ff9b9a}

#define TOUCH_TEST_BUFFER_CTL_CODE(id)  \
    CTL_CODE(FILE_DEVICE_KEYBOARD, (id), METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_TOUCH_SELFTEST_READ           TOUCH_TEST_BUFFER_CTL_CODE(100)
#define IOCTL_TOUCH_SELFTEST_WRITE          TOUCH_TEST_BUFFER_CTL_CODE(101)
#define IOCTL_TOUCH_SELFTEST_MODE           TOUCH_TEST_BUFFER_CTL_CODE(102)
#define IOCTL_TOUCH_SELFTEST_CHANGE_PAGE    TOUCH_TEST_BUFFER_CTL_CODE(103)

typedef struct _TOUCH_TEST_I2C_HEADER
{
    UCHAR AddressLength;
    UCHAR Address;
    ULONG RequestedTransferLength;
} TOUCH_TEST_I2C_HEADER;

EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL TchSelfTestOnDeviceControl;

EVT_WDF_DEVICE_FILE_CREATE TchSelfTestOnCreate;

EVT_WDF_FILE_CLOSE TchSelfTestOnClose;

NTSTATUS
TchSelfTestInitialize(
    IN WDFDEVICE Device
    );

