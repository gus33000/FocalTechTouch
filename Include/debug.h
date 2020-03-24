#include <wdm.h>

#define Trace(Level, Flags, Msg, ...) \
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "FT: " Msg "\n", __VA_ARGS__);

//#define COORDS_DEBUG
//#define ALS_BACKLIGHT_DEBUG

typedef enum _TraceFlags
{
    TRACE_FLAG_INIT = 1,
    TRACE_FLAG_REGISTRY,
    TRACE_FLAG_HID,
    TRACE_FLAG_PNP,
    TRACE_FLAG_POWER,
    TRACE_FLAG_SPB,
    TRACE_FLAG_CONFIG,
    TRACE_FLAG_REPORTING,
    TRACE_FLAG_INTERRUPT,
    TRACE_FLAG_SAMPLES,
    TRACE_FLAG_OTHER,
    TRACE_FLAG_IDLE
} TraceFlag;

typedef enum _TraceLevels
{
    TRACE_LEVEL_ERROR = 1,
    TRACE_LEVEL_VERBOSE,
    TRACE_LEVEL_INFORMATION,
    TRACE_LEVEL_WARNING
} TraceLevel;


