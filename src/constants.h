
#ifndef OMNIGLASS_LIB_CONSTANTS
    #define OMNIGLASS_LIB_CONSTANTS

/**operation results summarize the aftermath of API calls (as in: registering a callback, masking an event...).*/
typedef enum {
OMNIGLASS_RESULT_SUCCESS,
OMNIGLASS_RESULT_MULTITOUCH_REPORT_FAILED,
OMNIGLASS_RESULT_NOMEM,
OMNIGLASS_RESULT_BOOTSTRAP_FAILED
} omniglass_operation_results;

/**specifies what type of event triggered the action or callback.*/
typedef enum {
    OMNIGLASS_EVENT_GESTURE_STARTED,
    OMNIGLASS_EVENT_GESTURE_CHANGED,
    OMNIGLASS_EVENT_GESTURE_ENDED
} omniglass_event_type;

/** defines the results of touchpad platform initialization.*/
typedef enum {
    OMNIGLASS_PLATFORM_INIT_SUCCESS,
    OMNIGLASS_PLATFORM_INIT_NO_TOUCHPAD,
    OMNIGLASS_PLATFORM_INIT_CONFIG_INVALID
} omniglass_init_result;

#endif
