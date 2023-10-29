/** \file constants.h
 *  \brief mostly enumerations for use in flow control
 */
#ifndef OMNIGLASS_LIB_CONSTANTS
    #define OMNIGLASS_LIB_CONSTANTS

/**operation results summarize the aftermath of API calls (as in: registering a callback, masking an event...).*/
typedef enum {
OMNIGLASS_RESULT_SUCCESS,
OMNIGLASS_RESULT_MULTITOUCH_REPORT_FAILED,  /**< the library has failed to acquire input data from the touchpad */
OMNIGLASS_RESULT_NOMEM, /**< out-of-memory, failed to allocate memory */
OMNIGLASS_RESULT_BOOTSTRAP_FAILED   /**< the library could not initialize */
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
    OMNIGLASS_PLATFORM_INIT_NO_TOUCHPAD, /**< touchpad could not be detected*/
    OMNIGLASS_PLATFORM_INIT_CONFIG_INVALID /**< settings were not accepted */
} omniglass_init_result;

typedef enum {
    OMNIGLASS_API_GESTURE_OK,
    OMNIGLASS_API_GESTURE_NOTSUPPORTED, /**< touchpad in use does not support this gesture*/
    OMNIGLASS_API_GESTURE_NOTIMPLEMENTED, /**< the library does not support this gesture */
} omniglass_api_result;

#endif
