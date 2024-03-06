
 /** \file omniglass.h
  *  \brief the omniglass API definition.
  * 
  * the omniglass API gives applications means to detect and probe for gesture events on touchpad devices.
  */

#include "constants.h"
/** opaque omniGlass structure used by all user-facing library calls */
struct omniglass;

/** make sure to call this initializer before anything else
 @param handle a pointer to a pointer of an omniglass handle.
 */
omniglass_operation_results omniglass_init(struct omniglass **handle);

/** this function must be called at ~100hz in the application's main loop in non-blocking mode,
 * or as soon as possible in a separate thread.
 @param handle the handle to omniglass
 */
int omniglass_step(struct omniglass *handle);

/* callback section (listen, remove listener, define gestures)
*/

typedef void (*omniglass_callback_slide)(double);
typedef void (*omniglass_callback_pressed)(int);
typedef void (*omniglass_callback_released)(int);
typedef void (*omniglass_callback_edge)(double);

int omniglass_register_callback(struct omniglass *handle, void (*callback) (), omniglass_touchpad_edge edge);

omniglass_gesture_operation_result omniglass_listen_gesture_slide(struct omniglass *handle, omniglass_callback_slide callback);
void omniglass_disable_gesture_slide(struct omniglass *handle);

omniglass_gesture_operation_result omniglass_listen_gesture_edge(struct omniglass *handle, omniglass_callback_slide callback, omniglass_touchpad_edge edge);
void omniglass_disable_gesture_edge(struct omniglass *handle);

omniglass_gesture_operation_result omniglass_listen_gesture_edge_left(struct omniglass *handle, omniglass_callback_slide callback);
omniglass_gesture_operation_result omniglass_listen_gesture_edge_right(struct omniglass *handle, omniglass_callback_slide callback);
omniglass_gesture_operation_result omniglass_listen_gesture_edge_top(struct omniglass *handle, omniglass_callback_slide callback);
omniglass_gesture_operation_result omniglass_listen_gesture_edge_bottom(struct omniglass *handle, omniglass_callback_slide callback);
