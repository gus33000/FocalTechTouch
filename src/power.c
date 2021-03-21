/*++
    Copyright (c) Microsoft Corporation. All Rights Reserved.
    Sample code. Dealpoint ID #843729.

    Module Name:

        power.c

    Abstract:

        Contains Synaptics power-on and power-off functionality

    Environment:

        Kernel mode

    Revision History:

--*/

#include <controller.h>
#include <ftinternal.h>
#include <spb.h>
//#include <debug.h>
#include <power.tmh>


NTSTATUS
TchWakeDevice(
    IN VOID* ControllerContext,
    IN SPB_CONTEXT* SpbContext
)
/*++

Routine Description:

   Enables multi-touch scanning

Arguments:

   ControllerContext - Touch controller context

   SpbContext - A pointer to the current i2c context

Return Value:

   NTSTATUS indicating success or failure

--*/
{
    FT5X_CONTROLLER_CONTEXT* controller;

    UNREFERENCED_PARAMETER(SpbContext);

    controller = (FT5X_CONTROLLER_CONTEXT*)ControllerContext;

    //
    // Check if we were already on
    //
    if (controller->DevicePowerState == PowerDeviceD0)
    {
        goto exit;
    }

    controller->DevicePowerState = PowerDeviceD0;

exit:

    return STATUS_SUCCESS;
}

NTSTATUS
TchStandbyDevice(
    IN VOID* ControllerContext,
    IN SPB_CONTEXT* SpbContext
)
/*++

Routine Description:

   Disables multi-touch scanning to conserve power

Arguments:

   ControllerContext - Touch controller context

   SpbContext - A pointer to the current i2c context

Return Value:

   NTSTATUS indicating success or failure

--*/
{
    FT5X_CONTROLLER_CONTEXT* controller;

    UNREFERENCED_PARAMETER(SpbContext);

    controller = (FT5X_CONTROLLER_CONTEXT*)ControllerContext;

    //
    // Interrupts are now disabled but the ISR may still be
    // executing, so grab the controller lock to ensure ISR
    // is finished touching HW and controller state.
    //
    WdfWaitLockAcquire(controller->ControllerLock, NULL);

    controller->DevicePowerState = PowerDeviceD3;

    //
    // Invalidate state
    //
    controller->HidQueueCount = 0;

    WdfWaitLockRelease(controller->ControllerLock);

    return STATUS_SUCCESS;
}