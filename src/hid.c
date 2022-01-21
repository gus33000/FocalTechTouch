/*++
	Copyright (c) Microsoft Corporation. All Rights Reserved.
	Copyright (c) Bingxing Wang. All Rights Reserved.
	Copyright (c) Roman Masanin. All Rights Reserved.
	Copyright (c) LumiaWoA authors. All Rights Reserved.

	Module Name:

		hid.c

	Abstract:

		Code for handling HID related requests

	Environment:

		Kernel mode

	Revision History:

--*/

#include <Cross Platform Shim\compat.h>
#include <internal.h>
#include <controller.h>
#include <ft5x\ftinternal.h>
#include <hid.h>
#include <hid.tmh>

const USHORT gOEMVendorID = 0x6674;    // "ft"
const USHORT gOEMProductID = 0x3578;    // "5x"
const USHORT gOEMVersionID = 3200;

const PWSTR gpwstrManufacturerID = L"FocalTech";
const PWSTR gpwstrProductID = L"5x06";
const PWSTR gpwstrSerialNumber = L"5x06";

//
// HID Report Descriptor for a touch device
//

const UCHAR gReportDescriptor[] = {
	FOCALTECH_FT5X_DIGITIZER_DIAGNOSTIC1,
	FOCALTECH_FT5X_DIGITIZER_DIAGNOSTIC2,
	FOCALTECH_FT5X_DIGITIZER_DIAGNOSTIC3,
	FOCALTECH_FT5X_DIGITIZER_DIAGNOSTIC4,
	FOCALTECH_FT5X_DIGITIZER_FINGER,
	FOCALTECH_FT5X_DIGITIZER_REPORTMODE,
	FOCALTECH_FT5X_DIGITIZER_KEYPAD,
	FOCALTECH_FT5X_DIGITIZER_STYLUS
};
const ULONG gdwcbReportDescriptor = sizeof(gReportDescriptor);

//
// HID Descriptor for a touch device
//
const HID_DESCRIPTOR gHidDescriptor =
{
	sizeof(HID_DESCRIPTOR),             //bLength
	HID_HID_DESCRIPTOR_TYPE,            //bDescriptorType
	HID_REVISION,                       //bcdHID
	0,                                  //bCountry - not localized
	1,                                  //bNumDescriptors
	{                                   //DescriptorList[0]
		HID_REPORT_DESCRIPTOR_TYPE,     //bReportType
		sizeof(gReportDescriptor)       //wReportLength
	}
};

NTSTATUS
TchSendReport(
	IN WDFQUEUE PingPongQueue,
	IN PHID_INPUT_REPORT hidReportFromDriver
)
{
	NTSTATUS status;
	WDFREQUEST request;
	PHID_INPUT_REPORT hidReportRequestBuffer;
	size_t hidReportRequestBufferLength;

	status = STATUS_SUCCESS;
	request = NULL;

	switch (hidReportFromDriver->ReportID)
	{
	case REPORTID_STYLUS:
	{
	Trace(
		TRACE_LEVEL_INFORMATION,
		TRACE_HID,
		"HID pen: "
		"Tip Switch = %d, "
		"Barrel Switch = %d, "
		"Invert = %d, "
		"Eraser = %d, "
		"In Range = %d, "
		"X = %d, "
		"Y = %d, "
		"Tip Pressure = %d, "
		"X Tilt = %d, "
		"Y Tilt = %d",
		hidReportFromDriver->PenReport.TipSwitch,
		hidReportFromDriver->PenReport.BarrelSwitch,
		hidReportFromDriver->PenReport.Invert,
		hidReportFromDriver->PenReport.Eraser,
		hidReportFromDriver->PenReport.InRange,
		hidReportFromDriver->PenReport.X,
		hidReportFromDriver->PenReport.Y,
		hidReportFromDriver->PenReport.TipPressure,
		hidReportFromDriver->PenReport.XTilt,
		hidReportFromDriver->PenReport.YTilt);
	break;
	}
	case REPORTID_FINGER:
	{
		Trace(
			TRACE_LEVEL_INFORMATION,
			TRACE_HID,
			"HID Finger: "
			"Contact Count = %d\n"
			"Tip Switch = %d, "
			"In Range = %d, "
			"Confidence = %d, "
			"Contact ID = %d, "
			"X = %d, "
			"Y = %d\n"
			"Tip Switch = %d, "
			"In Range = %d, "
			"Confidence = %d, "
			"Contact ID = %d, "
			"X = %d, "
			"Y = %d",
			hidReportFromDriver->TouchReport.ContactCount,
			hidReportFromDriver->TouchReport.Contacts[0].TipSwitch,
			hidReportFromDriver->TouchReport.Contacts[0].InRange,
			hidReportFromDriver->TouchReport.Contacts[0].Confidence,
			hidReportFromDriver->TouchReport.Contacts[0].ContactID,
			hidReportFromDriver->TouchReport.Contacts[0].X,
			hidReportFromDriver->TouchReport.Contacts[0].Y,
			hidReportFromDriver->TouchReport.Contacts[1].TipSwitch,
			hidReportFromDriver->TouchReport.Contacts[1].InRange,
			hidReportFromDriver->TouchReport.Contacts[1].Confidence,
			hidReportFromDriver->TouchReport.Contacts[1].ContactID,
			hidReportFromDriver->TouchReport.Contacts[1].X,
			hidReportFromDriver->TouchReport.Contacts[1].Y);
		break;
	}
	case REPORTID_KEYPAD:
	{
		Trace(
			TRACE_LEVEL_INFORMATION,
			TRACE_HID,
			"HID key: "
			"System Power Down = %d, "
			"Start = %d, "
			"AC Search = %d, "
			"AC Back = %d",
			hidReportFromDriver->KeyReport.SystemPowerDown,
			hidReportFromDriver->KeyReport.Start,
			hidReportFromDriver->KeyReport.ACSearch,
			hidReportFromDriver->KeyReport.ACBack);
	}
	}

	//
	// Complete a HIDClass request if one is available
	//
	status = WdfIoQueueRetrieveNextRequest(
		PingPongQueue,
		&request);

	if (!NT_SUCCESS(status))
	{
		Trace(
			TRACE_LEVEL_ERROR,
			TRACE_REPORTING,
			"No request pending from HIDClass, ignoring report - 0x%08lX",
			status);

		goto exit;
	}

	//
	// Validate an output buffer was provided
	//
	status = WdfRequestRetrieveOutputBuffer(
		request,
		sizeof(HID_INPUT_REPORT),
		&hidReportRequestBuffer,
		&hidReportRequestBufferLength);

	if (!NT_SUCCESS(status))
	{
		Trace(
			TRACE_LEVEL_VERBOSE,
			TRACE_SAMPLES,
			"Error retrieving HID read request output buffer - 0x%08lX",
			status);
	}
	else
	{
		//
		// Validate the size of the output buffer
		//
		if (hidReportRequestBufferLength < sizeof(HID_INPUT_REPORT))
		{
			status = STATUS_BUFFER_TOO_SMALL;

			Trace(
				TRACE_LEVEL_VERBOSE,
				TRACE_SAMPLES,
				"Error HID read request buffer is too small (%I64x bytes) - 0x%08lX",
				hidReportRequestBufferLength,
				status);
		}
		else
		{
			RtlCopyMemory(
				hidReportRequestBuffer,
				hidReportFromDriver,
				sizeof(HID_INPUT_REPORT));

			WdfRequestSetInformation(request, sizeof(HID_INPUT_REPORT));
		}
	}

	WdfRequestComplete(request, status);

exit:
	return status;
}

NTSTATUS
TchReadReport(
	IN WDFDEVICE Device,
	IN WDFREQUEST Request,
	OUT BOOLEAN *Pending
)
/*++

Routine Description:

   Handles read requests from HIDCLASS, by forwarding the request.

Arguments:

   Device - Handle to WDF Device Object

   Request - Handle to request object

   Pending - flag to monitor if the request was sent down the stack

Return Value:

   On success, the function returns STATUS_SUCCESS
   On failure it passes the relevant error code to the caller.

--*/
{
	PDEVICE_EXTENSION devContext;
	NTSTATUS status;

	devContext = GetDeviceContext(Device);

	status = WdfRequestForwardToIoQueue(
		Request,
		devContext->ReportContext.PingPongQueue);

	if (!NT_SUCCESS(status))
	{
		Trace(
			TRACE_LEVEL_ERROR,
			TRACE_HID,
			"Failed to forward HID request to I/O queue - 0x%08lX",
			status);

		goto exit;
	}

	if (NULL != Pending)
	{
		*Pending = TRUE;
	}

	//
	// Service any interrupt that may have asserted while the framework had
	// interrupts disabled, or occurred before a read request was queued.
	//
	if (devContext->ServiceInterruptsAfterD0Entry == TRUE)
	{
		Ft5xServiceInterrupts(
			devContext->TouchContext,
			&devContext->I2CContext,
			&devContext->ReportContext);

		devContext->ServiceInterruptsAfterD0Entry = FALSE;
	}

exit:

	return status;
}

NTSTATUS
TchGetString(
	IN WDFDEVICE Device,
	IN WDFREQUEST Request
)
/*++

Routine Description:

	Returns string requested by the HIDCLASS driver

Arguments:

	Device - Handle to WDF Device Object

	Request - Handle to request object

Return Value:

	NTSTATUS indicating success or failure

--*/
{
	PIRP irp;
	PIO_STACK_LOCATION irpSp;
	ULONG_PTR lenId;
	NTSTATUS status;
	PWSTR strId;

	UNREFERENCED_PARAMETER(Device);

	status = STATUS_SUCCESS;

	irp = WdfRequestWdmGetIrp(Request);
	irpSp = IoGetCurrentIrpStackLocation(irp);
	switch ((ULONG_PTR)irpSp->Parameters.DeviceIoControl.Type3InputBuffer &
		0xffff)
	{
	case HID_STRING_ID_IMANUFACTURER:
		strId = gpwstrManufacturerID;
		break;

	case HID_STRING_ID_IPRODUCT:
		strId = gpwstrProductID;
		break;

	case HID_STRING_ID_ISERIALNUMBER:
		strId = gpwstrSerialNumber;
		break;

	default:
		strId = NULL;
		break;
	}

	lenId = strId ? (wcslen(strId)*sizeof(WCHAR) + sizeof(UNICODE_NULL)) : 0;
	if (strId == NULL)
	{
		status = STATUS_INVALID_PARAMETER;
	}
	else if (irpSp->Parameters.DeviceIoControl.OutputBufferLength < lenId)
	{
		status = STATUS_BUFFER_TOO_SMALL;
	}
	else
	{
		RtlCopyMemory(irp->UserBuffer, strId, lenId);
		irp->IoStatus.Information = lenId;
	}

	if (!NT_SUCCESS(status))
	{
		Trace(
			TRACE_LEVEL_ERROR,
			TRACE_HID,
			"Error getting device string - 0x%08lX",
			status);
	}

	return status;
}

NTSTATUS
TchGenerateHidReportDescriptor(
	IN WDFDEVICE Device,
	IN WDFMEMORY Memory
)
{
	PDEVICE_EXTENSION devContext;
	FT5X_CONTROLLER_CONTEXT* touchContext;
	NTSTATUS status;

	devContext = GetDeviceContext(Device);

	touchContext = (FT5X_CONTROLLER_CONTEXT*)devContext->TouchContext;

	PUCHAR hidReportDescBuffer = (PUCHAR)ExAllocatePoolWithTag(
		NonPagedPool,
		gdwcbReportDescriptor,
		TOUCH_POOL_TAG
	);

	if (hidReportDescBuffer == NULL)
	{
		Trace(
			TRACE_LEVEL_INFORMATION,
			TRACE_HID,
			"Failed to create hidReportDescBuffer on %p",
			hidReportDescBuffer);

		return STATUS_FATAL_MEMORY_EXHAUSTION;
	}

	RtlCopyBytes(
		hidReportDescBuffer,
		gReportDescriptor,
		gdwcbReportDescriptor
	);

	for (unsigned int i = 0; i < gdwcbReportDescriptor - 2; i++)
	{
		if (*(hidReportDescBuffer + i) == LOGICAL_MAXIMUM_2)
		{
			if (*(hidReportDescBuffer + i + 1) == 0xFE &&
				*(hidReportDescBuffer + i + 2) == 0xFE)
			{
				*(hidReportDescBuffer + i + 1) = devContext->ReportContext.Props.DisplayPhysicalWidth & 0xFF;
				*(hidReportDescBuffer + i + 2) = (devContext->ReportContext.Props.DisplayPhysicalWidth >> 8) & 0xFF;
			}
			if (*(hidReportDescBuffer + i + 1) == 0xFD &&
				*(hidReportDescBuffer + i + 2) == 0xFD)
			{
				*(hidReportDescBuffer + i + 1) = devContext->ReportContext.Props.DisplayPhysicalHeight & 0xFF;
				*(hidReportDescBuffer + i + 2) = (devContext->ReportContext.Props.DisplayPhysicalHeight >> 8) & 0xFF;
			}
		}
		else if (*(hidReportDescBuffer + i) == PHYSICAL_MAXIMUM_2)
		{
			if (*(hidReportDescBuffer + i + 1) == 0xFE &&
				*(hidReportDescBuffer + i + 2) == 0xFE)
			{
				*(hidReportDescBuffer + i + 1) = devContext->ReportContext.Props.DisplayWidth10um & 0xFF;
				*(hidReportDescBuffer + i + 2) = (devContext->ReportContext.Props.DisplayWidth10um >> 8) & 0xFF;
			}
			if (*(hidReportDescBuffer + i + 1) == 0xFD &&
				*(hidReportDescBuffer + i + 2) == 0xFD)
			{
				*(hidReportDescBuffer + i + 1) = devContext->ReportContext.Props.DisplayHeight10um & 0xFF;
				*(hidReportDescBuffer + i + 2) = (devContext->ReportContext.Props.DisplayHeight10um >> 8) & 0xFF;
			}
		}
	}

	//
	// Use hardcoded Report descriptor
	//
	status = WdfMemoryCopyFromBuffer(
		Memory,
		0,
		(PVOID)hidReportDescBuffer,
		gdwcbReportDescriptor);

	if (!NT_SUCCESS(status))
	{
		Trace(
			TRACE_LEVEL_ERROR,
			TRACE_HID,
			"Error copying HID report descriptor to request memory - STATUS:%X",
			status);
		goto exit;
	}

exit:
	ExFreePoolWithTag((PVOID)hidReportDescBuffer, TOUCH_POOL_TAG);
	return status;
}

NTSTATUS
TchGetHidDescriptor(
	IN WDFDEVICE Device,
	IN WDFREQUEST Request
)
/*++

Routine Description:

	Finds the HID descriptor and copies it into the buffer provided by the
	Request.

Arguments:

	Device - Handle to WDF Device Object

	Request - Handle to request object

Return Value:

	NTSTATUS indicating success or failure

--*/
{
	WDFMEMORY memory;
	NTSTATUS status;

	UNREFERENCED_PARAMETER(Device);

	//
	// This IOCTL is METHOD_NEITHER so WdfRequestRetrieveOutputMemory
	// will correctly retrieve buffer from Irp->UserBuffer. 
	// Remember that HIDCLASS provides the buffer in the Irp->UserBuffer
	// field irrespective of the ioctl buffer type. However, framework is very
	// strict about type checking. You cannot get Irp->UserBuffer by using
	// WdfRequestRetrieveOutputMemory if the ioctl is not a METHOD_NEITHER
	// internal ioctl.
	//
	status = WdfRequestRetrieveOutputMemory(Request, &memory);

	if (!NT_SUCCESS(status))
	{
		Trace(
			TRACE_LEVEL_ERROR,
			TRACE_HID,
			"Error getting HID descriptor request memory - 0x%08lX",
			status);
		goto exit;
	}

	//
	// Use hardcoded global HID Descriptor
	//
	status = WdfMemoryCopyFromBuffer(
		memory,
		0,
		(PUCHAR) &gHidDescriptor,
		sizeof(gHidDescriptor));

	if (!NT_SUCCESS(status))
	{
		Trace(
			TRACE_LEVEL_ERROR,
			TRACE_HID,
			"Error copying HID descriptor to request memory - 0x%08lX",
			status);
		goto exit;
	}

	//
	// Report how many bytes were copied
	//
	WdfRequestSetInformation(Request, sizeof(gHidDescriptor));

exit:

	return status;
}

NTSTATUS
TchGetReportDescriptor(
	IN WDFDEVICE Device,
	IN WDFREQUEST Request
)
/*++

Routine Description:

	Finds the report descriptor and copies it into the buffer provided by the
	Request.

Arguments:

	Device - Handle to WDF Device Object

	Request - Handle to request object

Return Value:

	NT status code.
	 success - STATUS_SUCCESS
	 failure:
	 STATUS_INVALID_PARAMETER - An invalid parameter was detected.

--*/
{
	WDFMEMORY memory;
	NTSTATUS status;

	//
	// This IOCTL is METHOD_NEITHER so WdfRequestRetrieveOutputMemory
	// will correctly retrieve buffer from Irp->UserBuffer. 
	// Remember that HIDCLASS provides the buffer in the Irp->UserBuffer
	// field irrespective of the ioctl buffer type. However, framework is very
	// strict about type checking. You cannot get Irp->UserBuffer by using
	// WdfRequestRetrieveOutputMemory if the ioctl is not a METHOD_NEITHER
	// internal ioctl.
	//
	status = WdfRequestRetrieveOutputMemory(Request, &memory);

	if (!NT_SUCCESS(status))
	{
		Trace(
			TRACE_LEVEL_ERROR,
			TRACE_HID,
			"Error getting HID report descriptor request memory - 0x%08lX",
			status);
		goto exit;
	}

	//
	// Use hardcoded Report descriptor
	//
	status = TchGenerateHidReportDescriptor(
		Device,
		memory
	);

	if (!NT_SUCCESS(status))
	{
		Trace(
			TRACE_LEVEL_ERROR,
			TRACE_HID,
			"Error copying HID report descriptor to request memory - 0x%08lX",
			status);
		goto exit;
	}

	//
	// Report how many bytes were copied
	//
	WdfRequestSetInformation(Request, gdwcbReportDescriptor);

exit:

	return status;
}

NTSTATUS
TchGetDeviceAttributes(
	IN WDFREQUEST Request
)
/*++

Routine Description:

	Fill in the given struct _HID_DEVICE_ATTRIBUTES

Arguments:

	Request - Pointer to Request object.

Return Value:

	NTSTATUS indicating success or failure

--*/
{
	PHID_DEVICE_ATTRIBUTES deviceAttributes;
	NTSTATUS status;

	//
	// This IOCTL is METHOD_NEITHER so WdfRequestRetrieveOutputMemory
	// will correctly retrieve buffer from Irp->UserBuffer. 
	// Remember that HIDCLASS provides the buffer in the Irp->UserBuffer
	// field irrespective of the ioctl buffer type. However, framework is very
	// strict about type checking. You cannot get Irp->UserBuffer by using
	// WdfRequestRetrieveOutputMemory if the ioctl is not a METHOD_NEITHER
	// internal ioctl.
	//
	status = WdfRequestRetrieveOutputBuffer(
		Request,
		sizeof (HID_DEVICE_ATTRIBUTES),
		&deviceAttributes,
		NULL);

	if (!NT_SUCCESS(status))
	{
		Trace(
			TRACE_LEVEL_ERROR,
			TRACE_HID,
			"Error retrieving device attribute output buffer - 0x%08lX",
			status);
		goto exit;
	}

	deviceAttributes->Size = sizeof (HID_DEVICE_ATTRIBUTES);
	deviceAttributes->VendorID = gOEMVendorID;
	deviceAttributes->ProductID = gOEMProductID;
	deviceAttributes->VersionNumber = gOEMVersionID;

	//
	// Report how many bytes were copied
	//
	WdfRequestSetInformation(Request, sizeof (HID_DEVICE_ATTRIBUTES));

exit:

	return status;
}

NTSTATUS
TchSetFeatureReport(
	IN WDFDEVICE Device,
	IN WDFREQUEST Request
)
/*++

Routine Description:

   Emulates setting a HID Feature report

Arguments:

   Device  - Framework device object
   Request - Framework request object

Return Value:

   NTSTATUS indicating success or failure

--*/
{
	PDEVICE_EXTENSION devContext;
	PHID_XFER_PACKET featurePacket;
	WDF_REQUEST_PARAMETERS params;
	NTSTATUS status;

	devContext = GetDeviceContext(Device);
	status = STATUS_SUCCESS;

	//
	// Validate Request Parameters
	//

	WDF_REQUEST_PARAMETERS_INIT(&params);
	WdfRequestGetParameters(Request, &params);

	if (params.Parameters.DeviceIoControl.InputBufferLength <
		sizeof(HID_XFER_PACKET))
	{
		status = STATUS_BUFFER_TOO_SMALL;
		goto exit;
	}

	featurePacket =
		(PHID_XFER_PACKET) WdfRequestWdmGetIrp(Request)->UserBuffer;

	if (featurePacket == NULL)
	{
		status = STATUS_INVALID_DEVICE_REQUEST;
		goto exit;
	}

	//
	// Process Request
	//

	switch (*(PUCHAR)featurePacket->reportBuffer)
	{
	case REPORTID_REPORTMODE:
	{
		Trace(
			TRACE_LEVEL_INFORMATION,
			TRACE_DRIVER,
			"%!FUNC! Report REPORTID_REPORTMODE is requested"
		);

		PPTP_DEVICE_INPUT_MODE_REPORT DeviceInputMode = (PPTP_DEVICE_INPUT_MODE_REPORT) featurePacket->reportBuffer;
		switch (DeviceInputMode->Mode)
		{
		case PTP_COLLECTION_MOUSE:
		{
			Trace(
				TRACE_LEVEL_INFORMATION,
				TRACE_DRIVER,
				"%!FUNC! Report REPORTID_REPORTMODE requested Mouse Input"
			);

			devContext->PtpInputOn = FALSE;
			break;
		}
		case PTP_COLLECTION_WINDOWS:
		{

			Trace(
				TRACE_LEVEL_INFORMATION,
				TRACE_DRIVER,
				"%!FUNC! Report REPORTID_REPORTMODE requested Windows PTP Input"
			);

			devContext->PtpInputOn = TRUE;
			break;
		}
		}

		Trace(
			TRACE_LEVEL_INFORMATION,
			TRACE_DRIVER,
			"%!FUNC! Report REPORTID_REPORTMODE is fulfilled"
		);
		break;
	}
	default:
	{
		Trace(
			TRACE_LEVEL_INFORMATION,
			TRACE_DRIVER,
			"%!FUNC! Unsupported type %d is requested",
			featurePacket->reportId
		);

		status = STATUS_NOT_SUPPORTED;
		goto exit;
	}
	}

exit:

	return status;
}

NTSTATUS
TchGetFeatureReport(
	IN WDFDEVICE Device,
	IN WDFREQUEST Request
)
/*++

Routine Description:

   Emulates retrieval of a HID Feature report

Arguments:

   Device  - Framework device object
   Request - Framework request object

Return Value:

   NTSTATUS indicating success or failure

--*/
{
	PDEVICE_EXTENSION devContext;
	PHID_XFER_PACKET featurePacket;
	WDF_REQUEST_PARAMETERS params;
	NTSTATUS status;
	size_t ReportSize;

	devContext = GetDeviceContext(Device);
	status = STATUS_SUCCESS;

	//
	// Validate Request Parameters
	//

	WDF_REQUEST_PARAMETERS_INIT(&params);
	WdfRequestGetParameters(Request, &params);

	if (params.Parameters.DeviceIoControl.OutputBufferLength <
		sizeof(HID_XFER_PACKET))
	{
		status = STATUS_BUFFER_TOO_SMALL;
		goto exit;
	}

	featurePacket =
		(PHID_XFER_PACKET) WdfRequestWdmGetIrp(Request)->UserBuffer;

	if (featurePacket == NULL)
	{
		status = STATUS_INVALID_DEVICE_REQUEST;
		goto exit;
	}

	//
	// Process Request
	//

	switch (*(PUCHAR)featurePacket->reportBuffer)
	{
	case REPORTID_DEVICE_CAPS:
	{
		Trace(
			TRACE_LEVEL_INFORMATION,
			TRACE_DRIVER,
			"%!FUNC! Report REPORTID_DEVICE_CAPS is requested"
		);

		// Size sanity check
		ReportSize = sizeof(PTP_DEVICE_CAPS_FEATURE_REPORT);
		if (featurePacket->reportBufferLen < ReportSize) {
			status = STATUS_INVALID_BUFFER_SIZE;
			Trace(
				TRACE_LEVEL_ERROR,
				TRACE_DRIVER,
				"%!FUNC! Report buffer is too small"
			);
			goto exit;
		}

		PPTP_DEVICE_CAPS_FEATURE_REPORT capsReport = (PPTP_DEVICE_CAPS_FEATURE_REPORT) featurePacket->reportBuffer;

		capsReport->MaximumContactPoints = PTP_MAX_CONTACT_POINTS;
		capsReport->ReportID = REPORTID_DEVICE_CAPS;

		if (devContext->TouchContext != NULL && ((FT5X_CONTROLLER_CONTEXT*)devContext->TouchContext)->MaxFingers != 0)
		{
			capsReport->MaximumContactPoints = ((FT5X_CONTROLLER_CONTEXT*)devContext->TouchContext)->MaxFingers;
		}

		Trace(
			TRACE_LEVEL_INFORMATION,
			TRACE_DRIVER,
			"%!FUNC! Report REPORTID_DEVICE_CAPS has maximum contact points of %d",
			capsReport->MaximumContactPoints
		);
		Trace(
			TRACE_LEVEL_INFORMATION,
			TRACE_DRIVER,
			"%!FUNC! Report REPORTID_DEVICE_CAPS is fulfilled"
		);

		break;
	}
	case REPORTID_PTPHQA:
	{
		Trace(
			TRACE_LEVEL_INFORMATION,
			TRACE_DRIVER,
			"%!FUNC! Report REPORTID_PTPHQA is requested"
		);

		// Size sanity check
		ReportSize = sizeof(PTP_DEVICE_HQA_CERTIFICATION_REPORT);
		if (featurePacket->reportBufferLen < ReportSize)
		{
			status = STATUS_INVALID_BUFFER_SIZE;
			Trace(
				TRACE_LEVEL_ERROR,
				TRACE_DRIVER,
				"%!FUNC! Report buffer is too small."
			);
			goto exit;
		}

		PPTP_DEVICE_HQA_CERTIFICATION_REPORT certReport = (PPTP_DEVICE_HQA_CERTIFICATION_REPORT) featurePacket->reportBuffer;

		*certReport->CertificationBlob = DEFAULT_PTP_HQA_BLOB;
		certReport->ReportID = REPORTID_PTPHQA;

		Trace(
			TRACE_LEVEL_INFORMATION,
			TRACE_DRIVER,
			"%!FUNC! Report REPORTID_PTPHQA is fulfilled"
		);

		break;
	}
	case REPORTID_PENHQA:
	{
		Trace(
			TRACE_LEVEL_INFORMATION,
			TRACE_DRIVER,
			"%!FUNC! Report REPORTID_PENHQA is requested"
		);

		// Size sanity check
		ReportSize = sizeof(PTP_DEVICE_HQA_CERTIFICATION_REPORT);
		if (featurePacket->reportBufferLen < ReportSize)
		{
			status = STATUS_INVALID_BUFFER_SIZE;
			Trace(
				TRACE_LEVEL_ERROR,
				TRACE_DRIVER,
				"%!FUNC! Report buffer is too small."
			);
			goto exit;
		}

		PPTP_DEVICE_HQA_CERTIFICATION_REPORT certReport = (PPTP_DEVICE_HQA_CERTIFICATION_REPORT)featurePacket->reportBuffer;

		*certReport->CertificationBlob = DEFAULT_PTP_HQA_BLOB;
		certReport->ReportID = REPORTID_PENHQA;

		Trace(
			TRACE_LEVEL_INFORMATION,
			TRACE_DRIVER,
			"%!FUNC! Report REPORTID_PENHQA is fulfilled"
		);

		break;
	}
	default:
	{
		Trace(
			TRACE_LEVEL_INFORMATION,
			TRACE_DRIVER,
			"%!FUNC! Unsupported type %d is requested",
			featurePacket->reportId
		);

		status = STATUS_NOT_SUPPORTED;
		goto exit;
	}
	}

exit:

	return status;
}
