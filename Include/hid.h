/*++
	Copyright (c) Microsoft Corporation. All Rights Reserved.
	Sample code. Dealpoint ID #843729.

	Module Name:

		hid.h

	Abstract:

		HID related function declarations, types, and defines

	Environment:

		Kernel mode

	Revision History:

--*/
#include "config.h"
#include "hidCommon.h"

#pragma once

#define FOCALTECH_TOUCH_DIGITIZER_FINGER_REPORT_COUNT 6

#define FOCALTECH_TOUCH_DIGITIZER_FINGER_COLLECTION \
        BEGIN_COLLECTION, 0x02,                 /*   COLLECTION (Logical) */ \
            LOGICAL_MAXIMUM, 0x01,                  /*     LOGICAL_MAXIMUM (1) */ \
            USAGE, 0x42,                            /*     USAGE (Tip Switch) */ \
            REPORT_COUNT, 0x01,                     /*     REPORT_COUNT (1) */ \
            REPORT_SIZE, 0x01,                      /*     REPORT_SIZE (1) */ \
            INPUT, 0x02,                            /*       INPUT (Data,Var,Abs) */ \
            USAGE, 0x32,                            /*     USAGE (In Range) */ \
            INPUT, 0x02,                            /*     INPUT (Data,Var,Abs) */ \
            REPORT_COUNT, 0x06,                     /*     REPORT_COUNT (6) */ \
            INPUT, 0x03,                            /*       INPUT (Cnst,Ary,Abs) */ \
            REPORT_SIZE, 0x08,                      /*     REPORT_SIZE (8) */ \
            USAGE, 0x51,                            /*     USAGE (Contact Identifier) */ \
            REPORT_COUNT, 0x01,                     /*     REPORT_COUNT (1) */ \
            INPUT, 0x02,                            /*       INPUT (Data,Var,Abs) */ \
                                                    \
		    USAGE_PAGE, 0x01,                       /* Usage Page: Generic Desktop */ \
		    LOGICAL_MAXIMUM_2, \
                    254, \
                    254, \
		    REPORT_SIZE, 0x10,                      /* Report Size: 0x10 (2 bytes) */ \
		    UNIT_EXPONENT, 0x0e,                    /* Unit exponent: -2 */ \
		    UNIT, 0x11,                             /* Unit: SI Length (cm) */ \
		    USAGE, 0x30,                            /* Usage: X */ \
		    PHYSICAL_MAXIMUM_2, 0xce, 0x02,         /* Physical Maximum: 7.18 */ \
		    REPORT_COUNT, 0x01,                     /* Report count: 1 */ \
		    INPUT, 0x02,                            /* Input: (Data, Var, Abs) */ \
		    PHYSICAL_MAXIMUM_2, 0xeb, 0x04,         /* Physical Maximum: 12.59 */ \
		    LOGICAL_MAXIMUM_2, \
                    253, \
                    253, \
		    USAGE, 0x31,                            /* Usage: Y */ \
		    INPUT, 0x02,                            /* Input: (Data, Var, Abs) */ \
		    PHYSICAL_MAXIMUM, 0x00,                 /* Physical Maximum: 0 */ \
		    UNIT_EXPONENT, 0x00,                    /* Unit exponent: 0 */ \
		    UNIT, 0x00,                             /* Unit: None */ \
        END_COLLECTION

#define FOCALTECH_TOUCH_DIGITIZER_COLLECTION \
		USAGE_PAGE, 0x0d,                       /*  USAGE_PAGE (Digitizers) */ \
		USAGE, 0x04,                            /*  USAGE (Touch Screen) */ \
		BEGIN_COLLECTION, 0x01,                 /*  COLLECTION (Application) */ \
			REPORT_ID, REPORTID_MTOUCH,                      /*    REPORT_ID (Touch) */ \
			FOCALTECH_TOUCH_DIGITIZER_FINGER_COLLECTION,     /*    Finger 1 */ \
			USAGE_PAGE, 0x0d,                                /*    USAGE_PAGE (Digitizers) */ \
			FOCALTECH_TOUCH_DIGITIZER_FINGER_COLLECTION,     /*    Finger 2 */ \
			USAGE_PAGE, 0x0d,                                /*    USAGE_PAGE (Digitizers) */ \
			FOCALTECH_TOUCH_DIGITIZER_FINGER_COLLECTION,     /*    Finger 2 */ \
			USAGE_PAGE, 0x0d,                                /*    USAGE_PAGE (Digitizers) */ \
			FOCALTECH_TOUCH_DIGITIZER_FINGER_COLLECTION,     /*    Finger 2 */ \
			USAGE_PAGE, 0x0d,                                /*    USAGE_PAGE (Digitizers) */ \
			FOCALTECH_TOUCH_DIGITIZER_FINGER_COLLECTION,     /*    Finger 2 */ \
			USAGE_PAGE, 0x0d,                                /*    USAGE_PAGE (Digitizers) */ \
			FOCALTECH_TOUCH_DIGITIZER_FINGER_COLLECTION,     /*    Finger 2 */ \
			USAGE_PAGE, 0x0d,                                /*    USAGE_PAGE (Digitizers) */ \
			USAGE, 0x54,                                     /*    USAGE (Actual count) */ \
			REPORT_COUNT, 0x01,                              /*    REPORT_COUNT (1) */ \
			REPORT_SIZE, 0x08,                               /*    REPORT_SIZE (8) */ \
			INPUT, 0x02,                                     /*      INPUT (Data,Var,Abs) */ \
			UNIT_EXPONENT, 0x0C,                             /*    UNIT_EXPONENT (-4) */ \
			UNIT_2, 0x01, 0x10,                              /*    UNIT (Seconds) */ \
			PHYSICAL_MAXIMUM_3, 0xff, 0xff, 0x00, 0x00,      /*    PHYSICAL_MAXIMUM (65535) */ \
			LOGICAL_MAXIMUM_3, 0xff, 0xff, 0x00, 0x00,       /*    LOGICAL_MAXIMUM (65535) */ \
			USAGE, 0x56,                                     /*    USAGE (Scan Time) */ \
			REPORT_COUNT, 0x01,                              /*    REPORT_COUNT (1) */ \
			REPORT_SIZE, 0x10,                               /*    REPORT_SIZE (16) */ \
			INPUT, 0x02,                                     /*      INPUT (Data,Var,Abs) */ \
			REPORT_ID, REPORTID_MAX_COUNT,                   /*    REPORT_ID (Feature) */ \
			USAGE, 0x55,                                     /*    USAGE(Maximum Count) */ \
			LOGICAL_MAXIMUM, 0x02,                           /*    LOGICAL_MAXIMUM (2) */ \
			FEATURE, 0x02,                                   /*    FEATURE (Data,Var,Abs) */ \
		END_COLLECTION

//
// Function prototypes
//

NTSTATUS
TchGetDeviceAttributes(
	IN WDFREQUEST Request
);

NTSTATUS
TchGetFeatureReport(
	IN WDFDEVICE Device,
	IN WDFREQUEST Request
);

NTSTATUS
TchGetHidDescriptor(
	IN WDFDEVICE Device,
	IN WDFREQUEST Request
);

NTSTATUS
TchGetReportDescriptor(
	IN WDFDEVICE Device,
	IN WDFREQUEST Request
);

NTSTATUS
TchGetString(
	IN WDFDEVICE Device,
	IN WDFREQUEST Request
);

NTSTATUS
TchProcessIdleRequest(
	IN  WDFDEVICE Device,
	IN  WDFREQUEST Request,
	OUT BOOLEAN* Pending
);

NTSTATUS
TchSetFeatureReport(
	IN WDFDEVICE Device,
	IN WDFREQUEST Request
);

NTSTATUS
TchReadReport(
	IN  WDFDEVICE Device,
	IN  WDFREQUEST Request,
	OUT BOOLEAN* Pending
);

