/*++
    Copyright (c) Microsoft Corporation. All Rights Reserved.
    Copyright (c) Bingxing Wang. All Rights Reserved.
    Copyright (c) LumiaWoA authors. All Rights Reserved.

    Module Name:

        enoselftest.c

    Abstract:

        Implements internal interfaces exposing functionality needed for
        controller self-tests, which can be used to run HW testing
        on a device in the manufacturing line, etc.

    Environment:

        Kernel mode

    Revision History:

--*/

#include <internal.h>
#include <controller.h>
#include <ft5x\ftinternal.h>
#include <spb.h>
#include <initguid.h>
#include <devguid.h>
#include <selftest\enoselftest.h>
#include <enoselftest.tmh>

VOID
TchEnoSelfTestOnDeviceControl(
    IN WDFQUEUE Queue,
    IN WDFREQUEST Request,
    IN size_t OutputBufferLength,
    IN size_t InputBufferLength,
    IN ULONG IoControlCode
)
/*++

Routine Description:

    This dispatch routine allows a user-mode application to issue test
    requests to the driver for execution on the chip and reporting of
    results.

Arguments:

    Queue - Framework queue object handle
    Request - Framework request object handle
    OutputBufferLength - self-explanatory
    InputBufferLength - self-explanatory
    IoControlCode - Specifies what is being requested

Return Value:

    NTSTATUS indicating success or failure

--*/
{
    PDEVICE_EXTENSION devContext;
    UCHAR* readBuffer = NULL;
    TOUCH_ENOTEST_I2C_HEADER* headerIn = NULL;
    TOUCH_ENOTEST_I2C_HEADER headerTemp = { 0 };
    size_t bytesReturned;
    NTSTATUS status = STATUS_INVALID_PARAMETER;
    BOOLEAN* requestedDiagnosticMode;
    UCHAR* requestedPage;


    devContext = GetDeviceContext(WdfPdoGetParent(WdfIoQueueGetDevice(Queue)));

    //
    // Ensure we're in a test session (framework should prevent this)
    //
    ASSERT(0 != devContext->TestSessionRefCnt);

    //
    // Process the test request
    //
    switch (IoControlCode)
    {
    case IOCTL_TOUCH_ENOSELFTEST_READ:
    {
        //
        // Validate parameters and memory
        //
        if (InputBufferLength != sizeof(TOUCH_ENOTEST_I2C_HEADER))
        {
            status = STATUS_INVALID_PARAMETER;
            goto exit;
        }

        status = WdfRequestRetrieveInputBuffer(
            Request,
            sizeof(TOUCH_ENOTEST_I2C_HEADER),
            (PVOID)&headerIn,
            NULL);

        if ((!NT_SUCCESS(status)) ||
            (headerIn->AddressLength != sizeof(headerIn->Address)) ||
            (headerIn->RequestedTransferLength < 1))
        {
            status = STATUS_INVALID_PARAMETER;
            goto exit;
        }

        //
        // Create a copy of headerIn since in and out buffers point to the
        // same memory and so SpbReadDataSync will overwrite it
        //
        headerTemp = *headerIn;

        status = WdfRequestRetrieveOutputBuffer(
            Request,
            headerTemp.RequestedTransferLength,
            (PVOID)&readBuffer,
            NULL);

        if ((!NT_SUCCESS(status)) ||
            (headerTemp.RequestedTransferLength > OutputBufferLength))
        {
            status = STATUS_INVALID_PARAMETER;
            goto exit;
        }

        //
        // Perform read
        //
        status = SpbReadDataSynchronously(
            &devContext->I2CContext,
            headerTemp.Address,
            readBuffer,
            headerTemp.RequestedTransferLength);
        if (!NT_SUCCESS(status))
        {
            goto exit;
        }

        WdfRequestSetInformation(Request, headerTemp.RequestedTransferLength);

        break;
    }

    case IOCTL_TOUCH_ENOSELFTEST_WRITE:
    {
        //
        // Validate parameters and memory
        //
        if (InputBufferLength < sizeof(TOUCH_ENOTEST_I2C_HEADER))
        {
            status = STATUS_INVALID_PARAMETER;
            goto exit;
        }

        status = WdfRequestRetrieveInputBuffer(
            Request,
            sizeof(TOUCH_ENOTEST_I2C_HEADER),
            (PVOID)&headerIn,
            &bytesReturned);

        if ((!NT_SUCCESS(status)) ||
            (headerIn->AddressLength != sizeof(headerIn->Address)) ||
            (bytesReturned != (sizeof(TOUCH_ENOTEST_I2C_HEADER) + headerIn->RequestedTransferLength)))
        {
            status = STATUS_INVALID_PARAMETER;
            goto exit;
        }

        //
        // Perform write
        //
        status = SpbWriteDataSynchronously(
            &devContext->I2CContext,
            headerIn->Address,
            (PVOID)(headerIn + 1),
            headerIn->RequestedTransferLength);

        if (!NT_SUCCESS(status))
        {
            goto exit;
        }

        WdfRequestSetInformation(Request, headerIn->RequestedTransferLength);

        break;
    }

    case IOCTL_TOUCH_ENOSELFTEST_MODE:
    {
        //
        // Validate parameters and memory
        //
        if (InputBufferLength != sizeof(BOOLEAN))
        {
            status = STATUS_INVALID_PARAMETER;
            goto exit;
        }

        status = WdfRequestRetrieveInputBuffer(
            Request,
            sizeof(BOOLEAN),
            (PVOID)&requestedDiagnosticMode,
            NULL);

        if (!NT_SUCCESS(status))
        {
            status = STATUS_INVALID_PARAMETER;
            goto exit;
        }

        if (*requestedDiagnosticMode != devContext->DiagnosticMode)
        {
            devContext->DiagnosticMode = *requestedDiagnosticMode;
        }

        WdfRequestSetInformation(Request, sizeof(*requestedDiagnosticMode));

        break;
    }

    case IOCTL_TOUCH_ENOSELFTEST_CHANGE_PAGE:
    {
        //
        // Validate parameters and memory
        //
        if (InputBufferLength != sizeof(UCHAR))
        {
            status = STATUS_INVALID_PARAMETER;
            goto exit;
        }

        status = WdfRequestRetrieveInputBuffer(
            Request,
            sizeof(UCHAR),
            (PVOID)&requestedPage,
            NULL);

        if (!NT_SUCCESS(status))
        {
            status = STATUS_INVALID_PARAMETER;
            goto exit;
        }

        status = Ft5xChangePage(
            devContext->TouchContext,
            &devContext->I2CContext,
            *requestedPage);
        if (!NT_SUCCESS(status))
        {
            goto exit;
        }

        WdfRequestSetInformation(Request, sizeof(*requestedPage));

        break;
    }

    default:
    {
        status = STATUS_NOT_IMPLEMENTED;
        break;
    }
    }

exit:

    WdfRequestComplete(
        Request,
        status);
}

VOID
TchEnoSelfTestOnCreate(
    IN WDFDEVICE Device,
    IN WDFREQUEST Request,
    IN WDFFILEOBJECT FileObject
)
/*++

Routine Description:

    This dispatch routine is invoked when a user-mode application is
    opening a test session. We simply reference count the number of
    creates and put the device into test mode.

Arguments:

    Device - Framework device object representing the test device
    Request - Can be used to get create information, ununsed here.
    FileObject - Unused here.

Return Value:

    None

--*/

{
    PDEVICE_EXTENSION devContext;
    LONG testSessionCount;

    UNREFERENCED_PARAMETER(FileObject);

    devContext = GetDeviceContext(WdfPdoGetParent(Device));
    testSessionCount = InterlockedIncrement(&(devContext->TestSessionRefCnt));

    WdfRequestComplete(
        Request,
        STATUS_SUCCESS);
}

VOID
TchEnoSelfTestOnClose(
    IN WDFFILEOBJECT FileObject
)
/*++

Routine Description:

    This dispatch routine is invoked when a user-mode application is
    closing a test session. We simply reference count the number of
    closes and restore the driver to normal operation if necessary.

Arguments:

    FileObject - Unused here.

Return Value:

    None

--*/

{
    PDEVICE_EXTENSION devContext;
    LONG testSessionCount;

    devContext = GetDeviceContext(WdfPdoGetParent(WdfFileObjectGetDevice(FileObject)));

    testSessionCount = InterlockedDecrement(&(devContext->TestSessionRefCnt));
}

NTSTATUS
TchEnoSelfTestInitialize(
    IN WDFDEVICE Device
)
/*++

Routine Description:

    This function creates a PDO that will be accessed directly by a user-mode
    application in order to initiate controller testing, gather results, etc.

    The test PDO is associated with the touch device's FDO and given a
    device interface so a user-mode application to find the test device.

    The function registers EvtIoDeviceControl, EvtIoRead, and EvtIoWrite
    dispatch routines which comprise the user-mode device driver interface for
    test functionality.

Arguments:

    Device - Framework device object representing the actual touch device

Return Value:

    NTSTATUS indicating success or failure

--*/
{
    NTSTATUS status;
    PDEVICE_EXTENSION devContext;
    PWDFDEVICE_INIT deviceInit = NULL;
    WDF_FILEOBJECT_CONFIG fileConfig;
    WDFDEVICE childDevice = NULL;
    WDF_OBJECT_ATTRIBUTES objectAttributes;
    WDF_IO_QUEUE_CONFIG queueConfig;

    DECLARE_CONST_UNICODE_STRING(deviceId, L"{1ED875DA-D851-42BE-9DFD-527D97178147}\\Touch Test\0");
    DECLARE_CONST_UNICODE_STRING(hardwareId, L"NOKIA_ENOTOUCHTEST");
    DECLARE_CONST_UNICODE_STRING(instanceId, L"0\0");
    DECLARE_CONST_UNICODE_STRING(instanceSecurity, L"D:P(A;;GA;;;SY)(A;;GRGWGX;;;BA)(A;;GR;;;WD)");

    devContext = GetDeviceContext(Device);

    //
    // Create a child test PDO, the touch device is the parent
    //
    deviceInit = WdfPdoInitAllocate(Device);

    if (NULL == deviceInit)
    {
        Trace(
            TRACE_LEVEL_ERROR,
            TRACE_INIT,
            "Error allocating WDFDEVICE_INIT structure");

        status = STATUS_INVALID_DEVICE_STATE;

        goto exit;
    }

    //
    // Assign security for this interface
    //
    status = WdfDeviceInitAssignSDDLString(
        deviceInit,
        &instanceSecurity);

    if (!NT_SUCCESS(status))
    {
        Trace(
            TRACE_LEVEL_ERROR,
            TRACE_INIT,
            "Error assigning test device object security - %!STATUS!",
            status);

        goto exit;
    }

    //
    // Indicate this PDO runs in "raw mode", so the framework doesn't
    // attempt to load a function driver -- we just want an interface
    // and non-HID stack for passing test data
    //
    status = WdfPdoInitAssignRawDevice(
        deviceInit,
        &GUID_DEVCLASS_HIDCLASS);

    if (!NT_SUCCESS(status))
    {
        Trace(
            TRACE_LEVEL_ERROR,
            TRACE_INIT,
            "Error assigning test device object as raw device - %!STATUS!",
            status);

        goto exit;
    }

    //
    // Assign device, hardware, instance IDs
    //
    status = WdfPdoInitAssignDeviceID(
        deviceInit,
        &deviceId);

    if (!NT_SUCCESS(status))
    {
        Trace(
            TRACE_LEVEL_ERROR,
            TRACE_INIT,
            "Error assigning test device ID - %!STATUS!",
            status);

        goto exit;
    }

    status = WdfPdoInitAddHardwareID(
        deviceInit,
        &hardwareId);

    if (!NT_SUCCESS(status))
    {
        Trace(
            TRACE_LEVEL_ERROR,
            TRACE_INIT,
            "Error assigning test device hardware ID - %!STATUS!",
            status);

        goto exit;
    }

    status = WdfPdoInitAssignInstanceID(
        deviceInit,
        &instanceId);

    if (!NT_SUCCESS(status))
    {
        Trace(
            TRACE_LEVEL_ERROR,
            TRACE_INIT,
            "Error assigning test device instance ID - %!STATUS!",
            status);

        goto exit;
    }

    //
    // We will want to know when a test application has opened
    // or closed a handle to the test device -- during this time
    // the touch driver's normal operation is interrupted.
    //
    WDF_FILEOBJECT_CONFIG_INIT(
        &fileConfig,
        TchEnoSelfTestOnCreate,
        TchEnoSelfTestOnClose,
        WDF_NO_EVENT_CALLBACK);

    WdfDeviceInitSetFileObjectConfig(
        deviceInit,
        &fileConfig,
        WDF_NO_OBJECT_ATTRIBUTES);

    //
    // Create the touch test device
    //
    WDF_OBJECT_ATTRIBUTES_INIT(&objectAttributes);

    status = WdfDeviceCreate(
        &deviceInit,
        &objectAttributes,
        &childDevice);

    if (!NT_SUCCESS(status))
    {
        Trace(
            TRACE_LEVEL_ERROR,
            TRACE_INIT,
            "Error creating test device - %!STATUS!",
            status);

        goto exit;
    }

    //
    // Set up an I/O request queue so that calling application
    // may send test requests to the device.
    //
    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(
        &queueConfig,
        WdfIoQueueDispatchParallel);

    queueConfig.EvtIoDeviceControl = TchEnoSelfTestOnDeviceControl;

    status = WdfIoQueueCreate(
        childDevice,
        &queueConfig,
        WDF_NO_OBJECT_ATTRIBUTES,
        &devContext->TestQueue);

    if (!NT_SUCCESS(status))
    {
        Trace(
            TRACE_LEVEL_ERROR,
            TRACE_INIT,
            "Error creating test device request queue - %!STATUS!",
            status);

        goto exit;
    }

    //
    // Expose a device interface for a user-mode test application
    // to access this test device
    //
    status = WdfDeviceCreateDeviceInterface(
        childDevice,
        &GUID_TOUCH_ENOSELFTEST_INTERFACE,
        NULL);

    if (!NT_SUCCESS(status))
    {
        Trace(
            TRACE_LEVEL_ERROR,
            TRACE_INIT,
            "Error creating test device interface - %!STATUS!",
            status);

        goto exit;
    }

    //
    // This must be called in addition to WdfPdoInitAllocate to
    // associate the test PDO just created as child of the touch
    // driver FDO
    //
    status = WdfFdoAddStaticChild(Device, childDevice);

    if (!NT_SUCCESS(status))
    {
        Trace(
            TRACE_LEVEL_ERROR,
            TRACE_INIT,
            "Error adding test PDO as child of touch FDO - %!STATUS!",
            status);

        goto exit;
    }

exit:

    if (!NT_SUCCESS(status))
    {
        if (deviceInit != NULL)
        {
            WdfDeviceInitFree(deviceInit);
        }

        if (childDevice != NULL)
        {
            WdfObjectDelete(childDevice);
        }
    }

    return status;
}
