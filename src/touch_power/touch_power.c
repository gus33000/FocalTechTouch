#include <controller.h>
#include <spb.h>
#include <internal.h>
#include <touch_power\public.h>
#include <touch_power\touch_power.h>
#include <touch_power.tmh>

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, PowerIoRegPnPNotification)
#endif

NTSTATUS
PowerToggle(
    TOUCH_POWER_CONTEXT* deviceContext,
    DWORD State
)
{
    NTSTATUS status = STATUS_SUCCESS;
    PUCHAR buffer;
    WDFMEMORY memory;
    WDF_MEMORY_DESCRIPTOR memoryDescriptor;
    ULONG_PTR bytesReturned = 0;

    memory = NULL;

    Trace(
        TRACE_LEVEL_INFORMATION,
        TRACE_POWER,
        "PowerToggle: Entry"
    );

    if (!deviceContext->TouchPowerOpen)
    {
        Trace(
            TRACE_LEVEL_INFORMATION,
            TRACE_POWER,
            "PowerToggle: Touch power is not open"
        );

        goto exit;
    }

    status = WdfMemoryCreate(
        WDF_NO_OBJECT_ATTRIBUTES,
        NonPagedPool,
        TOUCH_POWER_POOL_TAG,
        sizeof(DWORD),
        &memory,
        &buffer);

    if (!NT_SUCCESS(status))
    {
        Trace(
            TRACE_LEVEL_ERROR,
            TRACE_POWER,
            "Error allocating memory for touch power ioctl send - 0x%08lX",
            status);
        goto exit;
    }

    WDF_MEMORY_DESCRIPTOR_INIT_HANDLE(
        &memoryDescriptor,
        memory,
        NULL);

    //
    // Copy desired state into the buffer
    //
    RtlCopyMemory(buffer, &State, sizeof(DWORD));

    status = WdfIoTargetSendIoctlSynchronously(
        deviceContext->TouchPowerIOTarget,
        WDF_NO_HANDLE,
        (ULONG)IOCTL_TOUCH_POWER_TOGGLE,
        &memoryDescriptor,
        NULL,
        NULL,
        &bytesReturned);

    if (!NT_SUCCESS(status))
    {
        Trace(
            TRACE_LEVEL_ERROR,
            TRACE_SPB,
            "Error sending ioctl to touch power - 0x%08lX",
            status);
        goto exit;
    }

exit:
    Trace(
        TRACE_LEVEL_INFORMATION,
        TRACE_POWER,
        "PowerToggle: Exit"
    );

    if (NULL != memory)
    {
        WdfObjectDelete(memory);
    }

    return status;
}

NTSTATUS
PowerIoRegPnPNotification(
    IN  PVOID NotificationStructure,
    IN  PVOID Context)
{
    NTSTATUS status = STATUS_SUCCESS;
    PDEVICE_EXTENSION deviceContext = (PDEVICE_EXTENSION)Context;
    PDEVICE_INTERFACE_CHANGE_NOTIFICATION NotificationStruct = (PDEVICE_INTERFACE_CHANGE_NOTIFICATION)NotificationStructure;

    WDF_IO_TARGET_OPEN_PARAMS openParams;

    PAGED_CODE();

    Trace(
        TRACE_LEVEL_INFORMATION,
        TRACE_POWER,
        "PowerIoRegPnPNotification: Entry"
    );

    if (NULL == deviceContext)
    {
        Trace(
            TRACE_LEVEL_ERROR,
            TRACE_POWER,
            "PowerIoRegPnPNotification: Context is NULL"
        );

        return STATUS_UNSUCCESSFUL;
    }

    if (IsEqualGUID(&NotificationStruct->InterfaceClassGuid, &GUID_TOUCH_POWER_INTERFACE))
    {
        status = WdfIoTargetCreate(
            deviceContext->FxDevice,
            WDF_NO_OBJECT_ATTRIBUTES,
            &deviceContext->TouchPowerContext.TouchPowerIOTarget
        );

        if (!NT_SUCCESS(status))
        {
            Trace(
                TRACE_LEVEL_ERROR,
                TRACE_POWER,
                "PowerIoRegPnPNotification: Creating IO Target to Touch Power driver failed"
            );

            deviceContext->TouchPowerContext.TouchPowerOpen = FALSE;
            goto exit;
        }

        WDF_IO_TARGET_OPEN_PARAMS_INIT_OPEN_BY_NAME(&openParams, NotificationStruct->SymbolicLinkName, STANDARD_RIGHTS_ALL);

        status = WdfIoTargetOpen(deviceContext->TouchPowerContext.TouchPowerIOTarget, &openParams);
        if (!NT_SUCCESS(status))
        {
            Trace(
                TRACE_LEVEL_ERROR,
                TRACE_POWER,
                "PowerIoRegPnPNotification: Opening IO Target to Touch Power driver failed"
            );

            deviceContext->TouchPowerContext.TouchPowerOpen = FALSE;
            goto exit;
        }

        deviceContext->TouchPowerContext.TouchPowerOpen = TRUE;
    }
    else
    {
        WdfIoTargetClose(deviceContext->TouchPowerContext.TouchPowerIOTarget);
        deviceContext->TouchPowerContext.TouchPowerOpen = FALSE;
    }

exit:
    Trace(
        TRACE_LEVEL_INFORMATION,
        TRACE_POWER,
        "PowerIoRegPnPNotification: Exit"
    );

    return status;
}

NTSTATUS
PowerInitialize(
    WDFDEVICE Device
)
{
    NTSTATUS status = STATUS_SUCCESS;
    PDEVICE_EXTENSION deviceContext = (PDEVICE_EXTENSION)GetDeviceContext(Device);

    Trace(
        TRACE_LEVEL_INFORMATION,
        TRACE_POWER,
        "PowerInitialize: Entry"
    );

    status = IoRegisterPlugPlayNotification(
        EventCategoryDeviceInterfaceChange,
        PNPNOTIFY_DEVICE_INTERFACE_INCLUDE_EXISTING_INTERFACES,
        (PVOID)&GUID_TOUCH_POWER_INTERFACE,
        WdfDriverWdmGetDriverObject(WdfDeviceGetDriver(Device)),
        PowerIoRegPnPNotification,
        deviceContext,
        &deviceContext->TouchPowerContext.TouchPowerNotify);

    if (!NT_SUCCESS(status))
    {
        goto exit;
    }

exit:
    Trace(
        TRACE_LEVEL_INFORMATION,
        TRACE_POWER,
        "PowerInitialize: Exit"
    );

    return status;
}

NTSTATUS
PowerDeInitialize(
    WDFDEVICE Device
)
{
    NTSTATUS status = STATUS_SUCCESS;
    PDEVICE_EXTENSION deviceContext = (PDEVICE_EXTENSION)GetDeviceContext(Device);

    Trace(
        TRACE_LEVEL_INFORMATION,
        TRACE_POWER,
        "PowerDeInitialize: Entry"
    );

    if (deviceContext->TouchPowerContext.TouchPowerOpen == TRUE)
    {
        WdfIoTargetClose(deviceContext->TouchPowerContext.TouchPowerIOTarget);
    }

    if (deviceContext->TouchPowerContext.TouchPowerNotify)
    {
        status = IoUnregisterPlugPlayNotificationEx(deviceContext->TouchPowerContext.TouchPowerNotify);
        if (!NT_SUCCESS(status))
        {
            goto exit;
        }
    }

exit:
    Trace(
        TRACE_LEVEL_INFORMATION,
        TRACE_POWER,
        "PowerDeInitialize: Exit"
    );

    return status;
}
