/*++
	Copyright (c) Microsoft Corporation. All Rights Reserved.
	Sample code. Dealpoint ID #843729.

	Module Name:

		hid.c

	Abstract:

		Code for handling HID related requests

	Environment:

		Kernel mode

	Revision History:

--*/

#include <internal.h>
#include <controller.h>
#include <config.h>
#include <hid.h>
//#include <debug.h>
#include <hidCommon.h>
#include <ftinternal.h>
#include <hid.tmh>

//
// HID Report Descriptor for a touch device
//
const UCHAR gReportDescriptor[] = {
	FOCALTECH_TOUCH_DIGITIZER_COLLECTION,

	USAGE, 0x0E,                            // USAGE (Configuration)
	BEGIN_COLLECTION, 0x01,                 // COLLECTION (Application)
		REPORT_ID, REPORTID_FEATURE,            //   REPORT_ID (Feature)
		USAGE, 0x22,                            //   USAGE (Finger)
		BEGIN_COLLECTION, 0x00,                 //   COLLECTION (physical)
			USAGE, 0x52,                            //     USAGE (Input Mode)
			USAGE, 0x53,                            //     USAGE (Device Index)
			LOGICAL_MINIMUM, 0x00,                  //     LOGICAL_MINIMUM (0)
			LOGICAL_MAXIMUM, 0x0a,                  //     LOGICAL_MAXIMUM (10)
			REPORT_SIZE, 0x08,                      //     REPORT_SIZE (8)
			REPORT_COUNT, 0x02,                     //     REPORT_COUNT (2)
			FEATURE, 0x02,                          //     FEATURE (Data,Var,Abs)
		END_COLLECTION,                         //   END_COLLECTION
	END_COLLECTION,                         // END_COLLECTION

#ifdef HID_MOUSE_PATH_SUPPORT
	USAGE_PAGE, 0x01,                       // USAGE_PAGE (Generic Desktop)
	USAGE, 0x02,                            // USAGE (Mouse)
	BEGIN_COLLECTION, 0x01,                 // COLLECTION (Application)
		REPORT_ID, REPORTID_MOUSE,              //   REPORT_ID (Mouse)
		USAGE, 0x01,                            //   USAGE (Pointer)
		BEGIN_COLLECTION, 0x00,                 //   COLLECTION (Physical)
			USAGE_PAGE, 0x09,                       //     USAGE_PAGE (Button)
			0x19, 0x01,                             //     USAGE_MINIMUM (Button 1)
			0x29, 0x02,                             //     USAGE_MAXIMUM (Button 2)
			LOGICAL_MINIMUM, 0x00,                  //     LOGICAL_MINIMUM (0)
			LOGICAL_MAXIMUM, 0x01,                  //     LOGICAL_MAXIMUM (1)
			REPORT_SIZE, 0x01,                      //     REPORT_SIZE (1)
			REPORT_COUNT, 0x02,                     //     REPORT_COUNT (2)
			INPUT, 0x02,                            //       INPUT (Data,Var,Abs)
			REPORT_COUNT, 0x06,                     //     REPORT_COUNT (6)
			INPUT, 0x03,                            //       INPUT (Cnst,Var,Abs)
			USAGE_PAGE, 0x01,                       //     USAGE_PAGE (Generic Desktop)
			USAGE, 0x30,                            //     USAGE (X)
			USAGE, 0x31,                            //     USAGE (Y)
			REPORT_SIZE, 0x10,                      //     REPORT_SIZE (16)
			REPORT_COUNT, 0x02,                     //     REPORT_COUNT (2)
			LOGICAL_MINIMUM, 0x00,                  //     LOGICAL_MINIMUM (0)
			LOGICAL_MAXIMUM_2, 0xff, 0x7f,          //     LOGICAL_MAXIMUM (32767)
			INPUT, 0x02,                            //       INPUT (Data,Var,Abs)
		END_COLLECTION,                         //   END_COLLECTION
	END_COLLECTION,                         // END_COLLECTION
#endif
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
TchGenerateHidReportDescriptor
(
	IN PDEVICE_EXTENSION Context,
	IN WDFMEMORY outMemory
)
{
	NTSTATUS status = 0;
	FT5X_CONTROLLER_CONTEXT* touchContext = (FT5X_CONTROLLER_CONTEXT*)Context->TouchContext;

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

	Trace(
		TRACE_LEVEL_INFORMATION,
		TRACE_HID,
		"Created hidReportDescBuffer on %p",
		hidReportDescBuffer);

	RtlCopyBytes(
		hidReportDescBuffer,
		gReportDescriptor,
		gdwcbReportDescriptor
	);

	for (unsigned int i = 0; i < gdwcbReportDescriptor - 2; i++)
	{
		if (*(hidReportDescBuffer + i) == LOGICAL_MAXIMUM_2)
		{
			if (*(hidReportDescBuffer + i + 1) == 254 &&
				*(hidReportDescBuffer + i + 2) == 254)
			{
				*(hidReportDescBuffer + i + 1) = touchContext->Props.DisplayPhysicalWidth & 0xff;
				*(hidReportDescBuffer + i + 2) = (touchContext->Props.DisplayPhysicalWidth >> 8) & 0xff;

				Trace(
					TRACE_LEVEL_INFORMATION,
					TRACE_HID,
					"Set X=%u in %p",
					touchContext->Props.DisplayPhysicalWidth,
					hidReportDescBuffer + i);
			}
			if (*(hidReportDescBuffer + i + 1) == 253 &&
				*(hidReportDescBuffer + i + 2) == 253)
			{
				*(hidReportDescBuffer + i + 1) = touchContext->Props.DisplayPhysicalHeight & 0xff;
				*(hidReportDescBuffer + i + 2) = (touchContext->Props.DisplayPhysicalHeight >> 8) & 0xff;

				Trace(
					TRACE_LEVEL_INFORMATION,
					TRACE_HID,
					"Set Y=%u in %p",
					touchContext->Props.DisplayPhysicalHeight,
					hidReportDescBuffer + i);
			}
		}
	}

	Trace(
		TRACE_LEVEL_INFORMATION,
		TRACE_HID,
		"Set X=%u and Y=%u in hidReportDescriptor",
		touchContext->Props.DisplayPhysicalWidth,
		touchContext->Props.DisplayPhysicalHeight);

	//
	// Use hardcoded Report descriptor
	//
	status = WdfMemoryCopyFromBuffer(
		outMemory,
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
TchReadReport(
	IN WDFDEVICE Device,
	IN WDFREQUEST Request,
	OUT BOOLEAN* Pending
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
		devContext->PingPongQueue);

	if (!NT_SUCCESS(status))
	{
		Trace(
			TRACE_LEVEL_ERROR,
			TRACE_HID,
			"Failed to forward HID request to I/O queue - STATUS:%X",
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
		HID_INPUT_REPORT* hidReports = NULL;
        int reportsCount;

			TchServiceInterrupts(
				devContext->TouchContext,
				&devContext->I2CContext,
				devContext->InputMode,
                &hidReports,
				&reportsCount);

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

	lenId = strId ? (wcslen(strId) * sizeof(WCHAR) + sizeof(UNICODE_NULL)) : 0;
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
			"Error getting device string - STATUS:%X",
			status);
	}

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
			"Error getting HID descriptor request memory - STATUS:%X",
			status);
		goto exit;
	}

	//
	// Use hardcoded global HID Descriptor
	//
	status = WdfMemoryCopyFromBuffer(
		memory,
		0,
		(PUCHAR)&gHidDescriptor,
		sizeof(gHidDescriptor));

	if (!NT_SUCCESS(status))
	{
		Trace(
			TRACE_LEVEL_ERROR,
			TRACE_HID,
			"Error copying HID descriptor to request memory - STATUS:%X",
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
			"Error getting HID report descriptor request memory - STATUS:%X",
			status);
		goto exit;
	}

	PDEVICE_EXTENSION devCtx = GetDeviceContext(Device);

	status = TchGenerateHidReportDescriptor(devCtx, memory);
	if (!NT_SUCCESS(status))
	{
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
		sizeof(HID_DEVICE_ATTRIBUTES),
		&deviceAttributes,
		NULL);

	if (!NT_SUCCESS(status))
	{
		Trace(
			TRACE_LEVEL_ERROR,
			TRACE_HID,
			"Error retrieving device attribute output buffer - STATUS:%X",
			status);
		goto exit;
	}

	deviceAttributes->Size = sizeof(HID_DEVICE_ATTRIBUTES);
	deviceAttributes->VendorID = gOEMVendorID;
	deviceAttributes->ProductID = gOEMProductID;
	deviceAttributes->VersionNumber = gOEMVersionID;

	//
	// Report how many bytes were copied
	//
	WdfRequestSetInformation(Request, sizeof(HID_DEVICE_ATTRIBUTES));

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
		(PHID_XFER_PACKET)WdfRequestWdmGetIrp(Request)->UserBuffer;

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
	case REPORTID_FEATURE:
	{
		PHID_FEATURE_REPORT inputModeReport =
			(PHID_FEATURE_REPORT)featurePacket->reportBuffer;

		if (featurePacket->reportBufferLen < sizeof(HID_FEATURE_REPORT))
		{
			status = STATUS_BUFFER_TOO_SMALL;
			goto exit;
		}

		if ((inputModeReport->InputMode == MODE_MOUSE) ||
			(inputModeReport->InputMode == MODE_MULTI_TOUCH))
		{
			devContext->InputMode = inputModeReport->InputMode;
		}
		else
		{
			status = STATUS_INVALID_PARAMETER;
			goto exit;
		}

		break;
	}

	default:
	{
		status = STATUS_INVALID_PARAMETER;
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
		(PHID_XFER_PACKET)WdfRequestWdmGetIrp(Request)->UserBuffer;

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
	case REPORTID_FEATURE:
	{
		PHID_FEATURE_REPORT inputModeReport =
			(PHID_FEATURE_REPORT)featurePacket->reportBuffer;

		if (featurePacket->reportBufferLen < sizeof(HID_FEATURE_REPORT))
		{
			status = STATUS_BUFFER_TOO_SMALL;
			goto exit;
		}

		inputModeReport->InputMode = devContext->InputMode;

		break;
	}

	case REPORTID_MAX_COUNT:
	{
		PHID_MAX_COUNT_REPORT maxCountReport =
			(PHID_MAX_COUNT_REPORT)featurePacket->reportBuffer;

		if (featurePacket->reportBufferLen < sizeof(HID_MAX_COUNT_REPORT))
		{
			status = STATUS_BUFFER_TOO_SMALL;
			goto exit;
		}

		maxCountReport->MaxCount = OEM_MAX_TOUCHES;
		if (devContext->TouchContext != NULL)
		{
			FT5X_CONTROLLER_CONTEXT* controllerContext = devContext->TouchContext;
			if (controllerContext->MaxFingers != 0)
			{
				maxCountReport->MaxCount = controllerContext->MaxFingers;
			}
		}

		break;
	}

	default:
	{
		status = STATUS_INVALID_PARAMETER;
		goto exit;
	}
	}

exit:

	return status;
}
