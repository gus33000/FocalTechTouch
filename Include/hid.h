// Copyright (c) Microsoft Corporation. All Rights Reserved. 
// Copyright (c) Bingxing Wang. All Rights Reserved. 

#pragma once

//
// Global Data Declarations
//
extern const PWSTR gpwstrManufacturerID;
extern const PWSTR gpwstrProductID;
extern const PWSTR gpwstrSerialNumber;
extern const USHORT gOEMVendorID;
extern const USHORT gOEMProductID;
extern const USHORT gOEMVersionID;

//
// Structures
//

// REPORTID_DEVICE_CAPS
typedef struct _PTP_DEVICE_CAPS_FEATURE_REPORT {
	UCHAR ReportID;
	UCHAR MaximumContactPoints;
} PTP_DEVICE_CAPS_FEATURE_REPORT, * PPTP_DEVICE_CAPS_FEATURE_REPORT;

// REPORTID_REPORTMODE
typedef struct _PTP_DEVICE_INPUT_MODE_REPORT {
	UCHAR ReportID;
	UCHAR Mode;
	UCHAR DeviceID;
} PTP_DEVICE_INPUT_MODE_REPORT, * PPTP_DEVICE_INPUT_MODE_REPORT;

// REPORTID_PTPHQA REPORTID_PENHQA
typedef struct _PTP_DEVICE_HQA_CERTIFICATION_REPORT {
	UCHAR ReportID;
	UCHAR CertificationBlob[256];
} PTP_DEVICE_HQA_CERTIFICATION_REPORT, * PPTP_DEVICE_HQA_CERTIFICATION_REPORT;

// 
// Type defintions
//

#pragma warning(push)
#pragma warning(disable:4201)  // (nameless struct/union)
#include <pshpack1.h>

// REPORTID_FINGER
#pragma pack(push)
#pragma pack(1)
typedef struct _HID_TOUCH_FINGER {
	UCHAR		TipSwitch : 1;
	UCHAR		InRange : 1;
	UCHAR		Confidence : 1;
	UCHAR		Padding : 5;
	UCHAR		ContactID;
	USHORT		X;
	USHORT		Y;
} HID_TOUCH_FINGER, * PHID_TOUCH_FINGER;
#pragma pack(pop)

typedef struct _HID_TOUCH_REPORT {
	HID_TOUCH_FINGER Contacts[2];
	UCHAR            ContactCount;
} HID_TOUCH_REPORT, * PHID_TOUCH_REPORT;

// REPORTID_KEYPAD
typedef struct _HID_KEY_REPORT {
	UCHAR  SystemPowerDown : 1;
	UCHAR  Start : 1;
	UCHAR  ACSearch : 1;
	UCHAR  ACBack : 1;
	UCHAR  rReserved : 4;
	UCHAR  bReserved;
	USHORT wReserved;
} HID_KEY_REPORT, * PHID_KEY_REPORT;

// REPORTID_STYLUS
#pragma pack(push)
#pragma pack(1)
typedef struct _HID_PEN_REPORT {
	UCHAR  TipSwitch : 1;
	UCHAR  BarrelSwitch : 1;
	UCHAR  Invert : 1;
	UCHAR  Eraser : 1;
	UCHAR  Reserved : 1;
	UCHAR  InRange : 1;
	UCHAR  Padding : 2;
	USHORT X;
	USHORT Y;
	USHORT TipPressure;
	USHORT XTilt;
	USHORT YTilt;
} HID_PEN_REPORT, * PHID_PEN_REPORT;
#pragma pack(pop)

typedef struct _HID_INPUT_REPORT
{
	UCHAR ReportID;
	union
	{
		HID_TOUCH_REPORT TouchReport;
		HID_PEN_REPORT   PenReport;
		HID_KEY_REPORT   KeyReport;
	};
#ifdef _TIMESTAMP_
	LARGE_INTEGER TimeStamp;
#endif
} HID_INPUT_REPORT, * PHID_INPUT_REPORT;

#include <poppack.h>
#pragma warning(pop)

//
// Function prototypes
//

NTSTATUS
TchSendReport(
	IN WDFQUEUE PingPongQueue,
	IN PHID_INPUT_REPORT hidReportFromDriver
);

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
    OUT BOOLEAN *Pending
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
    OUT BOOLEAN *Pending
    );

//
// HID collections
// 
#include "HidCommon.h"

#define X_MASK 0xFE, 0xFE
#define Y_MASK 0xFD, 0xFD

#define FOCALTECH_FT5X_DIGITIZER_FINGER_CONTACT_1 \
	BEGIN_COLLECTION, 0x02, /* Collection (Logical) */ \
		USAGE, 0x42, /* Usage (Tip Switch) */ \
		LOGICAL_MINIMUM, 0x00, /* Logical Minimum (0) */ \
		LOGICAL_MAXIMUM, 0x01, /* Logical Maximum (1) */ \
		PHYSICAL_MINIMUM, 0x00, /* Physical Minimum (0) */ \
		PHYSICAL_MAXIMUM, 0x01, /* Physical Maximum (1) */ \
		UNIT, 0x00, /* Unit (None) */ \
		UNIT_EXPONENT, 0x00, /* Unit Exponent (0) */ \
		REPORT_SIZE, 0x01, /* Report Size (1) */ \
		REPORT_COUNT, 0x01, /* Report Count (1) */ \
		INPUT, 0x02, /* Input: (Data, Var, Abs) */ \
		USAGE, 0x32, /* Usage (In Range) */ \
		INPUT, 0x02, /* Input: (Data, Var, Abs) */ \
		USAGE, 0x47, /* Usage (Confidence) */ \
		INPUT, 0x02, /* Input: (Data, Var, Abs) */ \
		REPORT_COUNT, 0x05, /* Report Count (5) */ \
		INPUT, 0x03, /* Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position) */ \
		USAGE, 0x51, /* Usage (Contract Identifier) */ \
		PHYSICAL_MAXIMUM, 0x00, /* Physical Maximum (0) */ \
		REPORT_SIZE, 0x08, /* Report Size (8) */ \
		REPORT_COUNT, 0x01, /* Report Count (1) */ \
		INPUT, 0x02, /* Input: (Data, Var, Abs) */ \
		USAGE_PAGE, 0x01, /* Usage Page (Generic Desktop Ctrls) */ \
		USAGE, 0x30, /* Usage (X) */ \
		LOGICAL_MAXIMUM_2, X_MASK, /* Logical Maximum (1440) */ \
		PHYSICAL_MAXIMUM_2, X_MASK, /* Physical Maximum: 7.056 */ \
		UNIT, 0x11, /* Unit (System: SI Linear, Length: Centimeter) */ \
		UNIT_EXPONENT, 0x0d, /* Unit Exponent: -3 */ \
		REPORT_SIZE, 0x10, /* Report Size (16) */ \
		INPUT, 0x02, /* Input: (Data, Var, Abs) */ \
		USAGE, 0x31, /* Usage (Y) */ \
		LOGICAL_MAXIMUM_2, Y_MASK, /* Logical Maximum (2560) */ \
		PHYSICAL_MAXIMUM_2, Y_MASK, /* Physical Maximum: 12.544 */ \
		INPUT, 0x02, /* Input: (Data, Var, Abs) */ \
		PHYSICAL_MAXIMUM, 0x00, /* Physical Maximum: 0 */ \
		UNIT_EXPONENT, 0x00, /* Unit exponent: 0 */ \
		UNIT, 0x00, /* Unit: None */ \
	END_COLLECTION /* End Collection */

#define FOCALTECH_FT5X_DIGITIZER_FINGER_CONTACT_2 \
	BEGIN_COLLECTION, 0x02, /* Collection (Logical) */ \
		USAGE_PAGE, 0x0D, /* Usage Page (Digitizer) */ \
		USAGE, 0x42, /* Usage (Tip Switch) */ \
		LOGICAL_MAXIMUM, 0x01, /* Logical Maximum (1) */ \
		PHYSICAL_MAXIMUM, 0x01, /* Physical Maximum (1) */ \
		UNIT, 0x00, /* Unit (None) */ \
		UNIT_EXPONENT, 0x00, /* Unit Exponent (0) */ \
		REPORT_SIZE, 0x01, /* Report Size (1) */ \
		INPUT, 0x02, /* Input: (Data, Var, Abs) */ \
		USAGE, 0x32, /* Usage (In Range) */ \
		INPUT, 0x02, /* Input: (Data, Var, Abs) */ \
		USAGE, 0x47, /* Usage (Confidence) */ \
		INPUT, 0x02, /* Input: (Data, Var, Abs) */ \
		REPORT_COUNT, 0x05, /* Report Count (5) */ \
		INPUT, 0x03, /* Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position) */ \
		USAGE, 0x51, /* Usage (Contract Identifier) */ \
		PHYSICAL_MAXIMUM_2, Y_MASK, /* Physical Maximum: 12.544 */ \
		UNIT, 0x11, /* Unit (System: SI Linear, Length: Centimeter) */ \
		UNIT_EXPONENT, 0x0d, /* Unit Exponent: -3 */ \
		REPORT_SIZE, 0x08, /* Report Size (8) */ \
		REPORT_COUNT, 0x01, /* Report Count (1) */ \
		INPUT, 0x02, /* Input: (Data, Var, Abs) */ \
		USAGE_PAGE, 0x01, /* Usage Page (Generic Desktop Ctrls) */ \
		USAGE, 0x30, /* Usage (X) */ \
		LOGICAL_MAXIMUM_2, X_MASK, /* Logical Maximum (1440) */ \
		PHYSICAL_MAXIMUM_2, X_MASK, /* Physical Maximum: 7.056 */ \
		REPORT_SIZE, 0x10, /* Report Size (16) */ \
		INPUT, 0x02, /* Input: (Data, Var, Abs) */ \
		USAGE, 0x31, /* Usage (Y) */ \
		LOGICAL_MAXIMUM_2, Y_MASK, /* Logical Maximum (2560) */ \
		PHYSICAL_MAXIMUM_2, Y_MASK, /* Physical Maximum: 12.544 */ \
		INPUT, 0x02, /* Input: (Data, Var, Abs) */ \
		PHYSICAL_MAXIMUM, 0x00, /* Physical Maximum: 0 */ \
		UNIT_EXPONENT, 0x00, /* Unit exponent: 0 */ \
		UNIT, 0x00, /* Unit: None */ \
	END_COLLECTION /* End Collection */

#define FOCALTECH_FT5X_DIGITIZER_STYLUS_CONTACT_1 \
	BEGIN_COLLECTION, 0x00, /* Collection (Physical) */ \
		USAGE, 0x42, /* Usage (Tip Switch) */ \
		LOGICAL_MINIMUM, 0x00, /* Logical Minimum (0) */ \
		LOGICAL_MAXIMUM, 0x01, /* Logical Maximum (1) */ \
		PHYSICAL_MINIMUM, 0x00, /* Physical Minimum (0) */ \
		PHYSICAL_MAXIMUM, 0x01, /* Physical Maximum (1) */ \
		UNIT, 0x00, /* Unit (None) */ \
		UNIT_EXPONENT, 0x00, /* Unit Exponent (0) */ \
		REPORT_SIZE, 0x01, /* Report Size (1) */ \
		REPORT_COUNT, 0x01, /* Report Count (1) */ \
		INPUT, 0x02, /* Input: (Data, Var, Abs) */ \
		USAGE, 0x44, /* Usage (Barrel Switch) */ \
		INPUT, 0x02, /* Input: (Data, Var, Abs) */ \
		USAGE, 0x3C, /* Usage (Invert) */ \
		INPUT, 0x02, /* Input: (Data, Var, Abs) */ \
		USAGE, 0x45, /* Usage (Eraser) */ \
		INPUT, 0x02, /* Input: (Data, Var, Abs) */ \
		INPUT, 0x03, /* Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position) */ \
		USAGE, 0x32, /* Usage (In Range) */ \
		INPUT, 0x02, /* Input: (Data, Var, Abs) */ \
		REPORT_COUNT, 0x02, /* Report Count (2) */ \
		INPUT, 0x03, /* Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position) */ \
		USAGE_PAGE, 0x01, /* Usage Page (Generic Desktop Ctrls) */ \
		USAGE, 0x30, /* Usage (X) */ \
		LOGICAL_MAXIMUM_2, X_MASK, /* Logical Maximum (1440) */ \
		PHYSICAL_MAXIMUM_2, X_MASK, /* Physical Maximum: 7.056 */ \
		UNIT, 0x11, /* Unit (System: SI Linear, Length: Centimeter) */ \
		UNIT_EXPONENT, 0x0D, /* Unit Exponent: -3 */ \
		REPORT_SIZE, 0x10, /* Report Size (16) */ \
		REPORT_COUNT, 0x01, /* Report Count (1) */ \
		INPUT, 0x02, /* Input: (Data, Var, Abs) */ \
		USAGE, 0x31, /* Usage (Y) */ \
		LOGICAL_MAXIMUM_2, Y_MASK, /* Logical Maximum (2560) */ \
		PHYSICAL_MAXIMUM_2, Y_MASK, /* Physical Maximum: 12.544 */ \
		INPUT, 0x02, /* Input: (Data, Var, Abs) */ \
		USAGE_PAGE, 0x0D, /* Usage Page (Digitizer) */ \
		USAGE, 0x30, /* Usage (Tip Pressure) */ \
		LOGICAL_MAXIMUM, 0xFF, /* Logical Maximum (-1) */ \
		INPUT, 0x02, /* Input: (Data, Var, Abs) */ \
		USAGE, 0x3D, /* Usage (X Tilt) */ \
		LOGICAL_MAXIMUM, 0x7F, /* Logical Maximum (127) */ \
		REPORT_SIZE, 0x08, /* Report Size (8) */ \
		INPUT, 0x02, /* Input: (Data, Var, Abs) */ \
		USAGE, 0x3E, /* Usage (Y Tilt) */ \
		INPUT, 0x02, /* Input: (Data, Var, Abs) */ \
		PHYSICAL_MAXIMUM, 0x00, /* Physical Maximum: 0 */ \
		UNIT_EXPONENT, 0x00, /* Unit exponent: 0 */ \
		UNIT, 0x00, /* Unit: None */ \
	END_COLLECTION /* End Collection */

#define FOCALTECH_FT5X_DIGITIZER_DIAGNOSTIC1 \
	USAGE_PAGE_1, 0x05, 0xFF, /* Usage Page (Vendor Defined 0xFF05) */ \
	USAGE, 0x01, /* Usage (0x01) */ \
	BEGIN_COLLECTION, 0x01, /* Collection (Application) */ \
		REPORT_ID, REPORTID_DIAGNOSTIC_1, /* Report ID (-13) */ \
		USAGE, 0x20, /* Usage (0x20) */ \
		LOGICAL_MINIMUM, 0x00, /* Logical Minimum (0) */ \
		PHYSICAL_MINIMUM, 0x00, /* Physical Minimum (0) */ \
		PHYSICAL_MAXIMUM, 0x00, /* Physical Maximum (0) */ \
		UNIT, 0x00, /* Unit (None) */ \
		UNIT_EXPONENT, 0x00, /* Unit Exponent (0) */ \
		REPORT_SIZE, 0x08, /* Report Size (8) */ \
		REPORT_COUNT, 0xC7, /* Report Count (-57) */ \
		INPUT, 0x02, /* Input: (Data, Var, Abs) */ \
		REPORT_ID, REPORTID_DIAGNOSTIC_FEATURE_1, /* Report ID (-15) */ \
		USAGE, 0x31, /* Usage (0x31) */ \
		REPORT_COUNT, 0x3E, /* Report Count (62) */ \
		FEATURE, 0x02, /* Feature: (Data, Var, Abs) */ \
	END_COLLECTION /* End Collection */

#define FOCALTECH_FT5X_DIGITIZER_DIAGNOSTIC2 \
	USAGE_PAGE_1, 0x05, 0xFF, /* Usage Page (Vendor Defined 0xFF05) */ \
	USAGE, 0x02, /* Usage (0x02) */ \
	BEGIN_COLLECTION, 0x01, /* Collection (Application) */ \
		REPORT_ID, REPORTID_DIAGNOSTIC_2, /* Report ID (-14) */ \
		USAGE, 0x21, /* Usage (0x21) */ \
		LOGICAL_MINIMUM, 0x00, /* Logical Minimum (0) */ \
		PHYSICAL_MINIMUM, 0x00, /* Physical Minimum (0) */ \
		PHYSICAL_MAXIMUM, 0x00, /* Physical Maximum (0) */ \
		UNIT, 0x00, /* Unit (None) */ \
		UNIT_EXPONENT, 0x00, /* Unit Exponent (0) */ \
		REPORT_SIZE, 0x08, /* Report Size (8) */ \
		REPORT_COUNT, 0x10, /* Report Count (16) */ \
		INPUT, 0x02, /* Input: (Data, Var, Abs) */ \
	END_COLLECTION /* End Collection */

#define FOCALTECH_FT5X_DIGITIZER_DIAGNOSTIC3 \
	USAGE_PAGE_1, 0x05, 0xFF, /* Usage Page (Vendor Defined 0xFF05) */ \
	USAGE, 0x03, /* Usage (0x03) */ \
	BEGIN_COLLECTION, 0x01, /* Collection (Application) */ \
		REPORT_ID, REPORTID_DIAGNOSTIC_3, /* Report ID (-12) */ \
		USAGE, 0x22, /* Usage (0x22) */ \
		LOGICAL_MINIMUM, 0x00, /* Logical Minimum (0) */ \
		PHYSICAL_MINIMUM, 0x00, /* Physical Minimum (0) */ \
		PHYSICAL_MAXIMUM, 0x00, /* Physical Maximum (0) */ \
		UNIT, 0x00, /* Unit (None) */ \
		UNIT_EXPONENT, 0x00, /* Unit Exponent (0) */ \
		REPORT_SIZE, 0x08, /* Report Size (8) */ \
		REPORT_COUNT, 0x06, /* Report Count (6) */ \
		INPUT, 0x02, /* Input: (Data, Var, Abs) */ \
	END_COLLECTION /* End Collection */

#define FOCALTECH_FT5X_DIGITIZER_DIAGNOSTIC4 \
	USAGE_PAGE_1, 0x05, 0xFF, /* Usage Page (Vendor Defined 0xFF05) */ \
	USAGE, 0x04, /* Usage (0x04) */ \
	BEGIN_COLLECTION, 0x01, /* Collection (Application) */ \
		REPORT_ID, REPORTID_DIAGNOSTIC_4, /* Report ID (-11) */ \
		USAGE, 0x41, /* Usage (0x41) */ \
		LOGICAL_MINIMUM, 0x00, /* Logical Minimum (0) */ \
		PHYSICAL_MINIMUM, 0x00, /* Physical Minimum (0) */ \
		PHYSICAL_MAXIMUM, 0x00, /* Physical Maximum (0) */ \
		UNIT, 0x00, /* Unit (None) */ \
		UNIT_EXPONENT, 0x00, /* Unit Exponent (0) */ \
		REPORT_SIZE, 0x08, /* Report Size (8) */ \
		REPORT_COUNT_2, 0xD4, 0x07, /* Report Count (2004) */ \
		INPUT, 0x02, /* Input: (Data, Var, Abs) */ \
		REPORT_ID, REPORTID_DIAGNOSTIC_FEATURE_4, /* Report ID (-10) */ \
		USAGE, 0x32, /* Usage (0x32) */ \
		REPORT_COUNT, 0x03, /* Report Count (3) */ \
		FEATURE, 0x02, /* Feature: (Data, Var, Abs) */ \
	END_COLLECTION /* End Collection */

#define FOCALTECH_FT5X_DIGITIZER_FINGER \
	USAGE_PAGE, 0x0D, /* Usage Page (Digitizer) */ \
	USAGE, 0x04, /* Usage (Touch Screen) */ \
	BEGIN_COLLECTION, 0x01, /* Collection (Application) */ \
		REPORT_ID, REPORTID_FINGER, /* Report ID (1) */ \
		USAGE, 0x22, /* Usage (Finger) */ \
		FOCALTECH_FT5X_DIGITIZER_FINGER_CONTACT_1, /* Finger Contact (1) */ \
		USAGE, 0x00, /* Usage (Undefined) */ \
		FOCALTECH_FT5X_DIGITIZER_FINGER_CONTACT_2, /* Finger Contact (2) */ \
		USAGE_PAGE, 0x0D, /* Usage Page (Digitizer) */ \
		USAGE, 0x54, /* Usage (Contact Count) */ \
		REPORT_SIZE, 0x08, /* Report Size (8) */ \
		INPUT, 0x02, /* Input: (Data, Var, Abs) */ \
		REPORT_ID, REPORTID_DEVICE_CAPS, /* Report ID (8) */ \
		USAGE, 0x55, /* Usage (Maximum Contacts) */ \
		LOGICAL_MAXIMUM, 0x02, /* Logical Maximum (2) */ \
		FEATURE, 0x02, /* Feature: (Data, Var, Abs) */ \
		USAGE_PAGE_1, 0x00, 0xff, \
		REPORT_ID, REPORTID_PTPHQA, \
		USAGE, 0xc5, \
		LOGICAL_MINIMUM, 0x00, \
		LOGICAL_MAXIMUM_2, 0xff, 0x00, \
		REPORT_SIZE, 0x08, \
		REPORT_COUNT_2, 0x00, 0x01, \
		FEATURE, 0x02, \
	END_COLLECTION /* End Collection */

#define FOCALTECH_FT5X_DIGITIZER_REPORTMODE \
	USAGE_PAGE, 0x0D, /* Usage Page (Digitizer) */ \
	USAGE, 0x0E, /* Usage (Configuration) */ \
	BEGIN_COLLECTION, 0x01, /* Collection (Application) */ \
		REPORT_ID, REPORTID_REPORTMODE, /* Report ID (7) */ \
		USAGE, 0x22, /* Usage (Finger) */ \
		BEGIN_COLLECTION, 0x00, /* Collection (Physical) */ \
			USAGE, 0x52, /* Usage (Input Mode) */ \
			LOGICAL_MINIMUM, 0x00, /* Logical Minimum (0) */ \
			LOGICAL_MAXIMUM, 0x0A, /* Logical Maximum (10) */ \
			PHYSICAL_MINIMUM, 0x00, /* Physical Minimum (0) */ \
			PHYSICAL_MAXIMUM_2, Y_MASK, /* Physical Maximum: 12.544 */ \
			UNIT, 0x11, /* Unit (System: SI Linear, Length: Centimeter) */ \
			UNIT_EXPONENT, 0x0d, /* Unit Exponent: -3 */ \
			REPORT_SIZE, 0x08, /* Report Size (8) */ \
			REPORT_COUNT, 0x01, /* Report Count (1) */ \
			FEATURE, 0x02, /* Feature: (Data, Var, Abs) */ \
			USAGE, 0x53, /* Usage (Device Identifier) */ \
			FEATURE, 0x02, /* Feature: (Data, Var, Abs) */ \
		END_COLLECTION, /* End Collection */ \
	END_COLLECTION /* End Collection */

#define FOCALTECH_FT5X_DIGITIZER_KEYPAD \
	USAGE_PAGE, 0x01, /* Usage Page (Generic Desktop Ctrls) */ \
	USAGE, 0x0D, /* Usage (Portable Device Control) */ \
	BEGIN_COLLECTION, 0x01, /* Collection (Application) */ \
		REPORT_ID, REPORTID_KEYPAD, /* Report ID (9) */ \
		\
		USAGE_PAGE, 0x01, /* USAGE_PAGE (Generic Desktop Page) */ \
		USAGE, 0x81, /* System Power Down */\
		LOGICAL_MINIMUM, 0x00, /* Logical Minimum (0) */ \
		LOGICAL_MAXIMUM, 0x01, /* Logical Maximum (1) */ \
		PHYSICAL_MINIMUM, 0x00, /* Physical Minimum (0) */ \
		PHYSICAL_MAXIMUM, 0x01, /* Physical Maximum (1) */ \
		UNIT, 0x00, /* Unit (None) */ \
		UNIT_EXPONENT, 0x00, /* Unit Exponent (0) */ \
		REPORT_SIZE, 0x01, /* Report Size (1) */ \
		REPORT_COUNT, 0x01, /* Report Count (1) */ \
		INPUT, 0x02, /* Input: (Data, Var, Abs) */ \
		\
		USAGE_PAGE, 0x07, /* USAGE_PAGE (Keyboard Page) */ \
		USAGE, 0xE3, /* Keyboard Left GUI */\
		LOGICAL_MINIMUM, 0x00, /* Logical Minimum (0) */ \
		LOGICAL_MAXIMUM, 0x01, /* Logical Maximum (1) */ \
		PHYSICAL_MINIMUM, 0x00, /* Physical Minimum (0) */ \
		PHYSICAL_MAXIMUM, 0x01, /* Physical Maximum (1) */ \
		UNIT, 0x00, /* Unit (None) */ \
		UNIT_EXPONENT, 0x00, /* Unit Exponent (0) */ \
		REPORT_SIZE, 0x01, /* Report Size (1) */ \
		REPORT_COUNT, 0x01, /* Report Count (1) */ \
		INPUT, 0x02, /* Input: (Data, Var, Abs) */ \
		\
		USAGE_PAGE, 0x0C, /* USAGE_PAGE (Consumer Page) */ \
		USAGE_2, 0x21, 0x02, /* AC Search */\
		LOGICAL_MINIMUM, 0x00, /* Logical Minimum (0) */ \
		LOGICAL_MAXIMUM, 0x01, /* Logical Maximum (1) */ \
		PHYSICAL_MINIMUM, 0x00, /* Physical Minimum (0) */ \
		PHYSICAL_MAXIMUM, 0x01, /* Physical Maximum (1) */ \
		UNIT, 0x00, /* Unit (None) */ \
		UNIT_EXPONENT, 0x00, /* Unit Exponent (0) */ \
		REPORT_SIZE, 0x01, /* Report Size (1) */ \
		REPORT_COUNT, 0x01, /* Report Count (1) */ \
		INPUT, 0x02, /* Input: (Data, Var, Abs) */ \
		\
		USAGE_PAGE, 0x0C, /* USAGE_PAGE (Consumer Page) */ \
		USAGE_2, 0x24, 0x02, /* AC Back */\
		LOGICAL_MINIMUM, 0x00, /* Logical Minimum (0) */ \
		LOGICAL_MAXIMUM, 0x01, /* Logical Maximum (1) */ \
		PHYSICAL_MINIMUM, 0x00, /* Physical Minimum (0) */ \
		PHYSICAL_MAXIMUM, 0x01, /* Physical Maximum (1) */ \
		UNIT, 0x00, /* Unit (None) */ \
		UNIT_EXPONENT, 0x00, /* Unit Exponent (0) */ \
		REPORT_SIZE, 0x01, /* Report Size (1) */ \
		REPORT_COUNT, 0x01, /* Report Count (1) */ \
		INPUT, 0x02, /* Input: (Data, Var, Abs) */ \
		\
		REPORT_COUNT, 0x1c, /* Report Count (28) */ \
		INPUT, 0x03, /* Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position) */ \
	END_COLLECTION /* End Collection */

#define FOCALTECH_FT5X_DIGITIZER_STYLUS \
	USAGE_PAGE, 0x0D, /* Usage Page (Digitizer) */ \
	USAGE, 0x02, /* Usage (Pen) */ \
	BEGIN_COLLECTION, 0x01, /* Collection (Application) */ \
		REPORT_ID, REPORTID_STYLUS, /* Report ID (11) */ \
		USAGE, 0x20, /* Usage (Stylus) */ \
		FOCALTECH_FT5X_DIGITIZER_STYLUS_CONTACT_1, /* Stylus (1) */ \
		USAGE_PAGE_1, 0x00, 0xff, \
		REPORT_ID, REPORTID_PENHQA, \
		USAGE, 0xc5, \
		LOGICAL_MINIMUM, 0x00, \
		LOGICAL_MAXIMUM_2, 0xff, 0x00, \
		REPORT_SIZE, 0x08, \
		REPORT_COUNT_2, 0x00, 0x01, \
		FEATURE, 0x02, \
	END_COLLECTION /* End Collection */

#define DEFAULT_PTP_HQA_BLOB \
	0xfc, 0x28, 0xfe, 0x84, 0x40, 0xcb, 0x9a, 0x87, \
	0x0d, 0xbe, 0x57, 0x3c, 0xb6, 0x70, 0x09, 0x88, \
	0x07, 0x97, 0x2d, 0x2b, 0xe3, 0x38, 0x34, 0xb6, \
	0x6c, 0xed, 0xb0, 0xf7, 0xe5, 0x9c, 0xf6, 0xc2, \
	0x2e, 0x84, 0x1b, 0xe8, 0xb4, 0x51, 0x78, 0x43, \
	0x1f, 0x28, 0x4b, 0x7c, 0x2d, 0x53, 0xaf, 0xfc, \
	0x47, 0x70, 0x1b, 0x59, 0x6f, 0x74, 0x43, 0xc4, \
	0xf3, 0x47, 0x18, 0x53, 0x1a, 0xa2, 0xa1, 0x71, \
	0xc7, 0x95, 0x0e, 0x31, 0x55, 0x21, 0xd3, 0xb5, \
	0x1e, 0xe9, 0x0c, 0xba, 0xec, 0xb8, 0x89, 0x19, \
	0x3e, 0xb3, 0xaf, 0x75, 0x81, 0x9d, 0x53, 0xb9, \
	0x41, 0x57, 0xf4, 0x6d, 0x39, 0x25, 0x29, 0x7c, \
	0x87, 0xd9, 0xb4, 0x98, 0x45, 0x7d, 0xa7, 0x26, \
	0x9c, 0x65, 0x3b, 0x85, 0x68, 0x89, 0xd7, 0x3b, \
	0xbd, 0xff, 0x14, 0x67, 0xf2, 0x2b, 0xf0, 0x2a, \
	0x41, 0x54, 0xf0, 0xfd, 0x2c, 0x66, 0x7c, 0xf8, \
	0xc0, 0x8f, 0x33, 0x13, 0x03, 0xf1, 0xd3, 0xc1, \
	0x0b, 0x89, 0xd9, 0x1b, 0x62, 0xcd, 0x51, 0xb7, \
	0x80, 0xb8, 0xaf, 0x3a, 0x10, 0xc1, 0x8a, 0x5b, \
	0xe8, 0x8a, 0x56, 0xf0, 0x8c, 0xaa, 0xfa, 0x35, \
	0xe9, 0x42, 0xc4, 0xd8, 0x55, 0xc3, 0x38, 0xcc, \
	0x2b, 0x53, 0x5c, 0x69, 0x52, 0xd5, 0xc8, 0x73, \
	0x02, 0x38, 0x7c, 0x73, 0xb6, 0x41, 0xe7, 0xff, \
	0x05, 0xd8, 0x2b, 0x79, 0x9a, 0xe2, 0x34, 0x60, \
	0x8f, 0xa3, 0x32, 0x1f, 0x09, 0x78, 0x62, 0xbc, \
	0x80, 0xe3, 0x0f, 0xbd, 0x65, 0x20, 0x08, 0x13, \
	0xc1, 0xe2, 0xee, 0x53, 0x2d, 0x86, 0x7e, 0xa7, \
	0x5a, 0xc5, 0xd3, 0x7d, 0x98, 0xbe, 0x31, 0x48, \
	0x1f, 0xfb, 0xda, 0xaf, 0xa2, 0xa8, 0x6a, 0x89, \
	0xd6, 0xbf, 0xf2, 0xd3, 0x32, 0x2a, 0x9a, 0xe4, \
	0xcf, 0x17, 0xb7, 0xb8, 0xf4, 0xe1, 0x33, 0x08, \
	0x24, 0x8b, 0xc4, 0x43, 0xa5, 0xe5, 0x24, 0xc2
