/*++
    Copyright (c) Microsoft Corporation. All Rights Reserved.
    Sample code. Dealpoint ID #843729.

    Module Name:

        enoselftest.h

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
DEFINE_GUID(GUID_TOUCH_ENOSELFTEST_INTERFACE,
    0x1ED875DA, 0xD851, 0x42BE, 0x9D, 0xFD, 0x52, 0x7D, 0x97, 0x17, 0x81, 0x47);
// {1ED875DA-D851-42BE-9DFD-527D97178147}

#define TOUCH_ENOTEST_BUFFER_CTL_CODE(id)  \
    CTL_CODE(FILE_DEVICE_KEYBOARD, (id), METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_TOUCH_ENOSELFTEST_READ           TOUCH_ENOTEST_BUFFER_CTL_CODE(100)
#define IOCTL_TOUCH_ENOSELFTEST_WRITE          TOUCH_ENOTEST_BUFFER_CTL_CODE(101)
#define IOCTL_TOUCH_ENOSELFTEST_MODE           TOUCH_ENOTEST_BUFFER_CTL_CODE(102)
#define IOCTL_TOUCH_ENOSELFTEST_CHANGE_PAGE    TOUCH_ENOTEST_BUFFER_CTL_CODE(103)

typedef struct _TOUCH_ENOTEST_I2C_HEADER
{
    UCHAR AddressLength;
    UCHAR Address;
    ULONG RequestedTransferLength;
} TOUCH_ENOTEST_I2C_HEADER;

EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL TchEnoSelfTestOnDeviceControl;

EVT_WDF_DEVICE_FILE_CREATE TchEnoSelfTestOnCreate;

EVT_WDF_FILE_CLOSE TchEnoSelfTestOnClose;

NTSTATUS
TchEnoSelfTestInitialize(
    IN WDFDEVICE Device
);

