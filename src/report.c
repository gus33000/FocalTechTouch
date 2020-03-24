/*++
	Copyright (c) Microsoft Corporation. All Rights Reserved.
	Sample code. Dealpoint ID #843729.

	Module Name:

		report.c

	Abstract:

		Contains Synaptics specific code for reporting samples

	Environment:

		Kernel mode

	Revision History:

--*/

#include "controller.h"
#include "config.h"
#include "ftinternal.h"
#include "spb.h"
#include "debug.h"
#include "hid.h"
//#include "report.tmh"

NTSTATUS
FtServiceTouchDataInterrupt(
	IN FT5X_CONTROLLER_CONTEXT* ControllerContext,
	IN SPB_CONTEXT* SpbContext,
	IN UCHAR InputMode
)
/*++
Routine Description:
	Called when a touch interrupt needs service. Because we fill HID reports
	with two touches at a time, if more than two touches were read from
	hardware, we may need to complete this request from local cached state.
Arguments:
	ControllerContext - Touch controller context
	SpbContext - A pointer to the current SPB context (I2C, etc)
	HidReport- Buffer to fill with a hid report if touch data is available
	InputMode - Specifies mouse, single-touch, or multi-touch reporting modes
	PendingTouches - Notifies caller if there are more touches to report, to
		complete reporting the full state of fingers on the screen
Return Value:
	NTSTATUS indicating whether or not the current hid report buffer was filled
	PendingTouches also indicates whether the caller should expect more than
		one request to be completed to indicate the full state of fingers on
		the screen
--*/
{
	NTSTATUS status;

	status = STATUS_SUCCESS;

	PFOCAL_TECH_EVENT_DATA event_data;

	event_data = ExAllocatePoolWithTag(
		NonPagedPool,
		sizeof(FOCAL_TECH_EVENT_DATA),
		TOUCH_POOL_TAG);

	if (event_data == NULL)
	{
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto exit;
	}

	status = SpbReadDataSynchronously(SpbContext, 0, event_data, sizeof(FOCAL_TECH_EVENT_DATA));

	if (!NT_SUCCESS(status))
	{
		Trace(
			TRACE_LEVEL_ERROR,
			TRACE_FLAG_SAMPLES,
			"Error. Can't GetTouches from controller - STATUS %x",
			status
		);

		goto free_buffer;
	}

	//
	// If no touches are present return that no data needed to be reported
	//
	if (event_data->NumberOfTouchPoints == 0)
	{
		status = STATUS_NO_DATA_DETECTED;
		goto free_buffer;
	}

	//
	// Single-finger and HID-mouse input modes not implemented
	//
	if (MODE_MULTI_TOUCH != InputMode)
	{
		Trace(
			TRACE_LEVEL_WARNING,
			TRACE_FLAG_SAMPLES,
			"Unable to report touches, only multitouch mode is supported");

		status = STATUS_NOT_IMPLEMENTED;
		goto free_buffer;
	}

	//
	// Fill report with the next touches
	//
	{
		PHID_INPUT_REPORT hidReport;
		status = GetNextHidReport(ControllerContext, &hidReport);
		if (!NT_SUCCESS(status))
		{
			Trace(
				TRACE_LEVEL_ERROR,
				TRACE_FLAG_HID,
				"can't get report queue slot [fillHidReport(touches)], status: %x",
				status
			);
			goto free_buffer;
		}
		hidReport->ReportID = REPORTID_MTOUCH;

		PHID_TOUCH_REPORT hidTouch = &(hidReport->TouchReport);
		//
		// There are only 16-bits for ScanTime, truncate it
		//
		hidTouch->InputReport.ScanTime = 0 & 0xFFFF;

		//
		// Report the count
		// We're sending touches using hybrid mode with 2 fingers in our
		// report descriptor. The first report must indicate the
		// total count of touch fingers detected by the digitizer.
		// The remaining reports must indicate 0 for the count.
		// The first report will have the TouchesReported integer set to 0
		// The others will have it set to something else.
		//
		hidTouch->InputReport.ActualCount = event_data->NumberOfTouchPoints;

		for (int i = 0; i < event_data->NumberOfTouchPoints; i++)
		{
			hidTouch->InputReport.Contacts[i].ContactId = (UCHAR)i;

			USHORT SctatchX = event_data->TouchData[i].PositionX_Low | (event_data->TouchData[i].PositionX_High << 8);
			USHORT ScratchY = event_data->TouchData[i].PositionY_Low | (event_data->TouchData[i].PositionY_High << 8);

			//
			// Perform per-platform x/y adjustments to controller coordinates
			//
			TchTranslateToDisplayCoordinates(
				&SctatchX,
				&ScratchY,
				&ControllerContext->Props);

			hidTouch->InputReport.Contacts[i].wXData = SctatchX;
			hidTouch->InputReport.Contacts[i].wYData = ScratchY;

			hidTouch->InputReport.Contacts[i].bStatus = FINGER_STATUS;

#ifdef COORDS_DEBUG
			Trace(
				TRACE_LEVEL_NOISE,
				TRACE_FLAG_REPORTING,
				"ActualCount %d, ContactId %u X %u Y %u Tip %u",
				hidTouch->InputReport.ActualCount,
				hidTouch->InputReport.Contacts[i].ContactId,
				hidTouch->InputReport.Contacts[i].wXData,
				hidTouch->InputReport.Contacts[i].wYData,
				hidTouch->InputReport.Contacts[i].bStatus
			);
#endif
		}
	}

free_buffer:
	ExFreePoolWithTag(
		event_data,
		TOUCH_POOL_TAG
	);

	//
	// Update the caller if we still have outstanding touches to report
	//

exit:

	return status;
}

NTSTATUS
TchServiceInterrupts(
	IN VOID* ControllerContext,
	IN SPB_CONTEXT* SpbContext,
	IN UCHAR InputMode,
    IN PHID_INPUT_REPORT* HidReports,
    IN int* HidReportsLength
)
/*++

Routine Description:

	This routine is called in response to an interrupt. The driver will
	service chip interrupts, and if data is available to report to HID,
	fill the Request object buffer with a HID report.

Arguments:

	ControllerContext - Touch controller context
	SpbContext - A pointer to the current i2c context
	HidReport - Pointer to a HID_INPUT_REPORT structure to report to the OS
	InputMode - Specifies mouse, single-touch, or multi-touch reporting modes
	ServicingComplete - Notifies caller if there are more reports needed to
		complete servicing interrupts coming from the hardware.

Return Value:

	NTSTATUS indicating whether or not the current HidReport has been filled

	ServicingComplete indicates whether or not a new report buffer is required
		to complete interrupt processing.
--*/
{
	NTSTATUS status = STATUS_NO_DATA_DETECTED;
	FT5X_CONTROLLER_CONTEXT* controller;

	UNREFERENCED_PARAMETER(InputMode);
	UNREFERENCED_PARAMETER(SpbContext);

	controller = (FT5X_CONTROLLER_CONTEXT*)ControllerContext;

	//
	// Grab a waitlock to ensure the ISR executes serially and is 
	// protected against power state transitions
	//
	WdfWaitLockAcquire(controller->ControllerLock, NULL);

	//
	// RmiServiceXXX routine will change status to STATUS_SUCCESS if there
	// is a HID report to process.
	//
	status = STATUS_UNSUCCESSFUL;

	//
	// Service a touch data event if indicated by hardware 
	//
	{
		status = FtServiceTouchDataInterrupt(
			ControllerContext,
			SpbContext,
			InputMode);

		//
		// report error
		//
		if (!NT_SUCCESS(status) && status != STATUS_NO_DATA_DETECTED)
		{
			Trace(
				TRACE_LEVEL_ERROR,
				TRACE_FLAG_INTERRUPT,
				"Error processing touch event - STATUS:%X",
				status);
		}
	}

	//
	// Add servicing for additional touch interrupts here
	//

//exit:
    
    *HidReports = controller->HidQueue;
    *HidReportsLength = controller->HidQueueCount;
    controller->HidQueueCount = 0;

	WdfWaitLockRelease(controller->ControllerLock);

	return status;
}


NTSTATUS
GetNextHidReport(
    IN FT5X_CONTROLLER_CONTEXT* ControllerContext,
    IN PHID_INPUT_REPORT* HidReport
)
{
    if(ControllerContext->HidQueueCount < MAX_REPORTS_IN_QUEUE)
    {
        *HidReport = &(ControllerContext->HidQueue[ControllerContext->HidQueueCount]);
        ControllerContext->HidQueueCount++;
        RtlZeroMemory(*HidReport, sizeof(HID_INPUT_REPORT));
    }
    else
    {
        return STATUS_NO_MEMORY;
    }

    return STATUS_SUCCESS;
}