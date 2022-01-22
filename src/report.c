/*++
	Copyright (c) Microsoft Corporation. All Rights Reserved.
	Copyright (c) Bingxing Wang. All Rights Reserved.
	Copyright (c) LumiaWoA authors. All Rights Reserved.

	Module Name:

		report.c

	Abstract:

		Contains FocalTech specific code for reporting samples

	Environment:

		Kernel mode

	Revision History:

--*/

#include <Cross Platform Shim\compat.h>
#include <controller.h>
#include <resolutions.h>
#include <hid.h>
#include <HidCommon.h>
#include <spb.h>
#include <report.h>
#include <report.tmh>

WDFTIMER  timerHandle;
PREPORT_CONTEXT cachedReportContext = NULL;
DETECTED_OBJECTS objectData;

NTSTATUS
ReportWakeup(
	IN PREPORT_CONTEXT ReportContext
)
{
	NTSTATUS status = STATUS_SUCCESS;
	HID_INPUT_REPORT HidReport;

	RtlZeroMemory(&HidReport, sizeof(HID_INPUT_REPORT));

	HidReport.ReportID = REPORTID_KEYPAD;
	HidReport.KeyReport.ACBack = ReportContext->ButtonCache.ButtonSlots[0];
	HidReport.KeyReport.Start = ReportContext->ButtonCache.ButtonSlots[1];
	HidReport.KeyReport.ACSearch = ReportContext->ButtonCache.ButtonSlots[2];
	HidReport.KeyReport.SystemPowerDown = 1;

	status = TchSendReport(ReportContext->PingPongQueue, &HidReport);

	if (!NT_SUCCESS(status))
	{
		Trace(
			TRACE_LEVEL_ERROR,
			TRACE_REPORTING,
			"Error sending hid report for wake up (1) - 0x%08lX",
			status);

		goto exit;
	}

	RtlZeroMemory(&HidReport, sizeof(HID_INPUT_REPORT));

	HidReport.ReportID = REPORTID_KEYPAD;
	HidReport.KeyReport.ACBack = ReportContext->ButtonCache.ButtonSlots[0];
	HidReport.KeyReport.Start = ReportContext->ButtonCache.ButtonSlots[1];
	HidReport.KeyReport.ACSearch = ReportContext->ButtonCache.ButtonSlots[2];
	HidReport.KeyReport.SystemPowerDown = 0;

	status = TchSendReport(ReportContext->PingPongQueue, &HidReport);

	if (!NT_SUCCESS(status))
	{
		Trace(
			TRACE_LEVEL_ERROR,
			TRACE_REPORTING,
			"Error sending hid report for wake up (2) - 0x%08lX",
			status);

		goto exit;
	}

exit:
	return status;
}

NTSTATUS
ReportKeypad(
	IN PREPORT_CONTEXT ReportContext,
	IN BOOLEAN Back,
	IN BOOLEAN Start,
	IN BOOLEAN Search
)
{
	NTSTATUS status = STATUS_SUCCESS;
	HID_INPUT_REPORT HidReport;

	RtlZeroMemory(&HidReport, sizeof(HID_INPUT_REPORT));

	HidReport.ReportID = REPORTID_KEYPAD;
	HidReport.KeyReport.ACBack = Back;
	HidReport.KeyReport.Start = Start;
	HidReport.KeyReport.ACSearch = Search;

	ReportContext->ButtonCache.ButtonSlots[0] = Back;
	ReportContext->ButtonCache.ButtonSlots[1] = Start;
	ReportContext->ButtonCache.ButtonSlots[2] = Search;
	HidReport.KeyReport.SystemPowerDown = 0;

	status = TchSendReport(ReportContext->PingPongQueue, &HidReport);

	if (!NT_SUCCESS(status))
	{
		Trace(
			TRACE_LEVEL_ERROR,
			TRACE_REPORTING,
			"Error sending hid report for keypad - 0x%08lX",
			status);

		goto exit;
	}

exit:
	return status;
}

NTSTATUS
ReportPen(
	IN PREPORT_CONTEXT ReportContext,
	IN BOOLEAN TipSwitch,
	IN BOOLEAN BarrelSwitch,
	IN BOOLEAN Invert,
	IN BOOLEAN Eraser,
	IN BOOLEAN InRange,
	IN USHORT  X,
	IN USHORT  Y,
	IN USHORT  TipPressure,
	IN USHORT  XTilt,
	IN USHORT  YTilt
)
{
	NTSTATUS status;
	HID_INPUT_REPORT HidReport;
	RtlZeroMemory(&HidReport, sizeof(HID_INPUT_REPORT));

	USHORT ScratchX = (USHORT)X;
	USHORT ScratchY = (USHORT)Y;

	//
	// Perform per-platform x/y adjustments to controller coordinates
	//
	TchTranslateToDisplayCoordinates(
		&ScratchX,
		&ScratchY,
		&ReportContext->Props);

	HidReport.ReportID = REPORTID_STYLUS;

	HidReport.PenReport.InRange = InRange;
	HidReport.PenReport.TipSwitch = TipSwitch;
	HidReport.PenReport.Eraser = Eraser;
	HidReport.PenReport.Invert = Invert;
	HidReport.PenReport.BarrelSwitch = BarrelSwitch;

	HidReport.PenReport.X = ScratchX;
	HidReport.PenReport.Y = ScratchY;
	HidReport.PenReport.TipPressure = TipPressure;

	HidReport.PenReport.XTilt = XTilt;
	HidReport.PenReport.YTilt = YTilt;

	status = TchSendReport(ReportContext->PingPongQueue, &HidReport);

	if (!NT_SUCCESS(status))
	{
		Trace(
			TRACE_LEVEL_ERROR,
			TRACE_REPORTING,
			"Error sending hid report for active pen - 0x%08lX",
			status);

		goto exit;
	}

exit:
	return status;
}

VOID
ReportUpdateLocalObjectCache(
	IN DETECTED_OBJECTS* Data,
	IN OBJECT_CACHE* Cache
)
/*++

Routine Description:

	This routine takes raw data reported by the FocalTech hardware and
	parses it to update a local cache of finger states. This routine manages
	removing lifted touches from the cache, and manages a map between the
	order of reported touches in hardware, and the order the driver should
	use in reporting.

Arguments:

	Data - A pointer to the new data returned from hardware
	Cache - A data structure holding various current finger state info

Return Value:

	None.

--*/
{
	int i, j;

	//
	// When hardware was last read, if any slots reported as lifted, we
	// must clean out the slot and old touch info. There may be new
	// finger data using the slot.
	//
	for (i = 0; i < MAX_TOUCHES; i++)
	{
		//
		// Sweep for a slot that needs to be cleaned
		//
		if (!(Cache->SlotDirty & (1 << i)))
		{
			continue;
		}

		NT_ASSERT(Cache->DownCount > 0);

		//
		// Find the slot in the reporting list 
		//
		for (j = 0; j < MAX_TOUCHES; j++)
		{
			if (Cache->DownOrder[j] == i)
			{
				break;
			}
		}

		NT_ASSERT(j != MAX_TOUCHES);

		//
		// Remove the slot. If the finger lifted was the last in the list,
		// we just decrement the list total by one. If it was not last, we
		// shift the trailing list items up by one.
		//
		for (; (j < Cache->DownCount - 1) && (j < MAX_TOUCHES - 1); j++)
		{
			Cache->DownOrder[j] = Cache->DownOrder[j + 1];
		}
		Cache->DownCount--;

		//
		// Finished, clobber the dirty bit
		//
		Cache->SlotDirty &= ~(1 << i);
	}

	//
	// Cache the new set of finger data reported by hardware
	//
	for (i = 0; i < MAX_TOUCHES; i++)
	{
		//
		// Take actions when a new contact is first reported as down
		//
		if ((Data->States[i] != OBJECT_STATE_NOT_PRESENT) &&
			((Cache->SlotValid & (1 << i)) == 0) &&
			(Cache->DownCount < MAX_TOUCHES))
		{
			Cache->SlotValid |= (1 << i);
			Cache->DownOrder[Cache->DownCount++] = i;
		}

		//
		// Ignore slots with no new information
		//
		if (!(Cache->SlotValid & (1 << i)))
		{
			continue;
		}

		//
		// When finger is down, update local cache with new information from
		// the controller. When finger is up, we'll use last cached value
		//
		Cache->Slot[i].status = (UCHAR)Data->States[i];
		if (Cache->Slot[i].status)
		{
			Cache->Slot[i].x = Data->Positions[i].X;
			Cache->Slot[i].y = Data->Positions[i].Y;
		}

		//
		// If a finger lifted, note the slot is now inactive so that any
		// cached data is cleaned out before we read hardware again.
		//
		if (Cache->Slot[i].status == OBJECT_STATE_NOT_PRESENT)
		{
			Cache->SlotDirty |= (1 << i);
			Cache->SlotValid &= ~(1 << i);
		}
	}

	//
	// Get current scan time (in 100us units)
	//
	ULONG64 QpcTimeStamp;
	Cache->ScanTime = KeQueryInterruptTimePrecise(&QpcTimeStamp) / 1000;
}

NTSTATUS
ReportObjectsInternal(
	IN PREPORT_CONTEXT ReportContext,
	IN DETECTED_OBJECTS data
)
/*++

Routine Description:

	Called when a touch interrupt needs service.

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
	NTSTATUS status = STATUS_SUCCESS;
	HID_INPUT_REPORT HidReport;
	int TouchesReported = 0;
	int currentFingerIndex;
	int fingersToReport = 0;
	USHORT SctatchX = 0, ScratchY = 0;
	BOOLEAN HasPen = FALSE;

	//
	// Process the new touch data by updating our cached state
	//
	ReportUpdateLocalObjectCache(
		&data,
		&ReportContext->Cache);

	//
	// If no touches are present return that no data needed to be reported
	//
	if (ReportContext->Cache.DownCount == 0)
	{
		status = STATUS_NO_DATA_DETECTED;
		goto exit;
	}

	while (TouchesReported != ReportContext->Cache.DownCount)
	{
		//
		// Fill report with the next cached touches
		//
		RtlZeroMemory(&HidReport, sizeof(HID_INPUT_REPORT));

		currentFingerIndex = 0;

		fingersToReport = min(ReportContext->Cache.DownCount - TouchesReported, 2);

		HidReport.ReportID = REPORTID_FINGER;

		//
		// There are only 16-bits for ScanTime, truncate it
		//
		//HidReport->ScanTime = Cache->ScanTime & 0xFFFF;

		//
		// Report the count
		// We're sending touches using hybrid mode with 5 fingers in our
		// report descriptor. The first report must indicate the
		// total count of touch fingers detected by the digitizer.
		// The remaining reports must indicate 0 for the count.
		// The first report will have the TouchesReported integer set to 0
		// The others will have it set to something else.
		//
		if (TouchesReported == 0)
		{
			HidReport.TouchReport.ContactCount = (UCHAR)ReportContext->Cache.DownCount;
		}
		else
		{
			HidReport.TouchReport.ContactCount = 0;
		}

		HasPen = FALSE;

		for (currentFingerIndex = 0; currentFingerIndex < fingersToReport; currentFingerIndex++)
		{
			int currentlyReporting = ReportContext->Cache.DownOrder[TouchesReported];

			OBJECT_INFO info = ReportContext->Cache.Slot[currentlyReporting];

			if (info.status == OBJECT_STATE_PEN_PRESENT_WITH_ERASER ||
				info.status == OBJECT_STATE_PEN_PRESENT_WITH_TIP)
			{
				HasPen = TRUE;
				ReportContext->PenPresent = TRUE;

				status = ReportPen(
					ReportContext,
					TRUE,
					FALSE,
					info.status == OBJECT_STATE_PEN_PRESENT_WITH_ERASER,
					info.status == OBJECT_STATE_PEN_PRESENT_WITH_ERASER,
					TRUE,
					(USHORT)info.x,
					(USHORT)info.y,
					1,
					0,
					0);
				if (!NT_SUCCESS(status))
				{
					Trace(
						TRACE_LEVEL_ERROR,
						TRACE_REPORTING,
						"Error sending hid report for passive pen - 0x%08lX",
						status);

					goto exit;
				}
			}

			HidReport.TouchReport.Contacts[currentFingerIndex].ContactID = (UCHAR)currentlyReporting;
			SctatchX = (USHORT)info.x;
			ScratchY = (USHORT)info.y;
			HidReport.TouchReport.Contacts[currentFingerIndex].Confidence = 1;

			//
			// Perform per-platform x/y adjustments to controller coordinates
			//
			TchTranslateToDisplayCoordinates(
				&SctatchX,
				&ScratchY,
				&ReportContext->Props);

			if (info.status == OBJECT_STATE_FINGER_PRESENT_WITH_ACCURATE_POS)
			{
				HidReport.TouchReport.Contacts[currentFingerIndex].X = SctatchX;
				HidReport.TouchReport.Contacts[currentFingerIndex].Y = ScratchY;
				HidReport.TouchReport.Contacts[currentFingerIndex].TipSwitch = FINGER_STATUS;
			}

			TouchesReported++;
		}

		if (HasPen == FALSE && ReportContext->PenPresent == TRUE)
		{
			ReportContext->PenPresent = FALSE;

			status = ReportPen(
				ReportContext,
				FALSE,
				FALSE,
				FALSE,
				FALSE,
				FALSE,
				0,
				0,
				0,
				0,
				0);

			if (!NT_SUCCESS(status))
			{
				Trace(
					TRACE_LEVEL_ERROR,
					TRACE_REPORTING,
					"Error sending hid report for passive pen - 0x%08lX",
					status);

				goto exit;
			}
		}

		status = TchSendReport(ReportContext->PingPongQueue, &HidReport);

		if (!NT_SUCCESS(status))
		{
			Trace(
				TRACE_LEVEL_ERROR,
				TRACE_REPORTING,
				"Error sending hid report for fingers - 0x%08lX",
				status);

			goto exit;
		}
	}

exit:
	return status;
}

NTSTATUS
TchContinuousObjectInterruptServicingEvtTimerFunc(
	IN WDFTIMER Timer
)
{
	NTSTATUS status = STATUS_SUCCESS;

	Trace(
            TRACE_LEVEL_ERROR,
		TRACE_REPORTING,
		"TchContinuousObjectInterruptServicingEvtTimerFunc ENTRY");

      if (cachedReportContext == NULL)
      {
		Trace(
			TRACE_LEVEL_ERROR,
			TRACE_REPORTING,
			"Error while reporting objects - cachedReportContext is NULL");

            WdfTimerStop(Timer, FALSE);
		goto exit;
      }

	status = ReportObjectsInternal(
		cachedReportContext,
		objectData);

	if (!NT_SUCCESS(status))
	{
		Trace(
			TRACE_LEVEL_ERROR,
			TRACE_REPORTING,
			"Error while reporting objects - 0x%08lX",
			status);

            WdfTimerStop(Timer, FALSE);
            status = STATUS_SUCCESS;
		goto exit;
	}

exit:
	Trace(
            TRACE_LEVEL_ERROR,
		TRACE_REPORTING,
		"TchContinuousObjectInterruptServicingEvtTimerFunc EXIT - 0x%08lX",
		status);

	return status;
}

NTSTATUS
ReportConfigureContinuousSimulationTimer(
	IN WDFDEVICE DeviceHandle
)
{
	NTSTATUS status = STATUS_SUCCESS;

	WDF_TIMER_CONFIG  timerConfig;
	WDF_OBJECT_ATTRIBUTES  timerAttributes;

	WDF_TIMER_CONFIG_INIT(
      	&timerConfig,
      	TchContinuousObjectInterruptServicingEvtTimerFunc);

      timerConfig.Period = 50;

      WDF_OBJECT_ATTRIBUTES_INIT(&timerAttributes);
      timerAttributes.ParentObject = DeviceHandle;

	status = WdfTimerCreate(
      	&timerConfig,
      	&timerAttributes,
      	&timerHandle);

	if (!NT_SUCCESS(status))
	{
		Trace(
			TRACE_LEVEL_ERROR,
			TRACE_INIT,
			"Error while creating the WDF timer - 0x%08lX",
			status);

		goto exit;
	}

exit:
	return status;
}

NTSTATUS
ReportObjectsContinuous(
	IN PREPORT_CONTEXT ReportContext,
	IN DETECTED_OBJECTS data
)
{
      NTSTATUS status = STATUS_SUCCESS;

	Trace(
            TRACE_LEVEL_ERROR,
		TRACE_REPORTING,
		"ReportObjectsContinuous ENTRY");

      WdfTimerStop(timerHandle, TRUE);

      cachedReportContext = ReportContext;

      RtlCopyMemory(&objectData, &data, sizeof(objectData));

	status = ReportObjectsInternal(
		ReportContext,
		objectData);

	if (!NT_SUCCESS(status))
	{
		Trace(
			TRACE_LEVEL_VERBOSE,
			TRACE_SAMPLES,
			"Error while reporting objects - 0x%08lX",
			status);

		goto exit;
	}

	WdfTimerStart(timerHandle, WDF_REL_TIMEOUT_IN_MS(50));

exit:	
      Trace(
            TRACE_LEVEL_ERROR,
		TRACE_REPORTING,
		"ReportObjectsContinuous EXIT - 0x%08lX",
		status);

	return status;
}

NTSTATUS
ReportObjects(
	IN PREPORT_CONTEXT ReportContext,
	IN DETECTED_OBJECTS data
)
{
	if (ReportContext->Props.TouchHardwareLacksContinuousReporting)
      {
            return ReportObjectsContinuous(
		      ReportContext,
		      data);
      }
      else
      {
            return ReportObjectsInternal(
		      ReportContext,
		      data);
      }
}