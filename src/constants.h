/** \file constants.h
 *  \brief mostly enumerations for use in flow control
 */
#ifndef OMNIGLASS_LIB_CONSTANTS
    #define OMNIGLASS_LIB_CONSTANTS

/**operation results summarize the aftermath of API calls (as in: registering a callback, masking an event...).*/
typedef enum {
OMNIGLASS_RESULT_SUCCESS = 0,
OMNIGLASS_RESULT_MULTITOUCH_REPORT_FAILED,  /**< the library has failed to acquire input data from the touchpad */
OMNIGLASS_RESULT_NOMEM, /**< out-of-memory, failed to allocate memory */
OMNIGLASS_RESULT_BOOTSTRAP_FAILED   /**< the library could not initialize */
} omniglass_operation_results;

/**specifies what type of event triggered the action or callback.*/
typedef enum {
    OMNIGLASS_EVENT_GESTURE_STARTED = 0,
    OMNIGLASS_EVENT_GESTURE_CHANGED,
    OMNIGLASS_EVENT_GESTURE_ENDED
} omniglass_event_type;

/**defines the results of touchpad platform initialization.*/
typedef enum {
    OMNIGLASS_PLATFORM_INIT_SUCCESS = 0,
    OMNIGLASS_PLATFORM_INIT_NO_TOUCHPAD, /**< touchpad could not be detected*/
    OMNIGLASS_PLATFORM_INIT_CONFIG_INVALID /**< settings were not accepted */
} omniglass_init_result;

/**results of gesture operations*/
typedef enum {
    OMNIGLASS_API_GESTURE_OPERATION_SUCCESS = 0,
    OMNIGLASS_API_GESTURE_OPERATION_NOTSUPPORTED, /**< touchpad in use does not support this gesture*/
    OMNIGLASS_API_GESTURE_OPERATION_NOTIMPLEMENTED, /**< the library does not support this gesture */
} omniglass_gesture_operation_result;

/**represents edges/borders of a touchpad.*/
typedef enum {
    OMNIGLASS_EDGE_LEFT = 0,
    OMNIGLASS_EDGE_RIGHT,
    OMNIGLASS_EDGE_TOP,
    OMNIGLASS_EDGE_BOTTOM
} omniglass_touchpad_edge;

// const char *omniglass_touchpad_edge_names [] = {
//     "left",
//     "right",
//     "top",
//     "bottom"
// };
#endif
