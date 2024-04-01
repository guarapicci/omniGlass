
 /** \file omniglass.h
  *  \brief the omniglass API definition.
  * 
  * the omniglass API gives applications means to detect and probe for gesture events on touchpad devices.
  */

#include "constants.h"
#include "config.h"
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
 * most callbacks accept a "passthrough" void pointer that the user can provide at register-time.
 * this allows passing to each callback the data it needs without using global variables.
*/

typedef void (*omniglass_callback_slide)(double);
typedef void (*omniglass_callback_pressed)(int);
typedef void (*omniglass_callback_released)(int);
typedef void (*omniglass_callback_edge)(double, void*);

int omniglass_register_callback(struct omniglass *handle, void (*callback) (), omniglass_touchpad_edge edge);

/** set a callback function for finger slide.
 * whenever at least one finger has been in contact accross the surface and its contact position has moved since the last call to omniglass_step(), this callback is triggered.
 * @param handle the handle to omniGlass.
 * @param callback the function that will be called when a slide has happen
 */
omniglass_gesture_operation_result omniglass_listen_gesture_slide(struct omniglass *handle, omniglass_callback_slide callback);
void omniglass_disable_gesture_slide(struct omniglass *handle);

/** schedule a function to be called whenever fingers have dragged accross a certain edge.
 * @param handle the omniglass handle.
 * @param callback this function is called upon edge scroll
 * @param edge one of the touchpad's 4 edges where the dragging will be scanned for
 * @param passthrough this arbitrary data is forwarded to the callback function.
 */
omniglass_gesture_operation_result omniglass_listen_gesture_edge(struct omniglass *handle, omniglass_callback_edge callback, omniglass_touchpad_edge edge, void *passthrough);

/** disable all edge scroll detection callbacks.
 * @param handle the omniglass handle.
 */
void omniglass_disable_gesture_edge(struct omniglass *handle);

omniglass_gesture_operation_result omniglass_listen_gesture_edge_left(struct omniglass *handle, omniglass_callback_edge callback, void * passthrough);
omniglass_gesture_operation_result omniglass_listen_gesture_edge_right(struct omniglass *handle, omniglass_callback_edge callback, void *passthrough);
omniglass_gesture_operation_result omniglass_listen_gesture_edge_top(struct omniglass *handle, omniglass_callback_edge callback, void *passthrough);
omniglass_gesture_operation_result omniglass_listen_gesture_edge_bottom(struct omniglass *handle, omniglass_callback_edge callback, void *passthrough);
