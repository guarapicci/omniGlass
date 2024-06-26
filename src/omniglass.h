
 /** \file omniglass.h
  *  \brief the omniglass public API definition.
  * 
  * the omniglass API gives applications means to detect and probe for gesture events on touchpad devices.
  */
#ifndef OMNIGLASS_PUBLIC_API_INCLUDED
#define OMNIGLASS_PUBLIC_API_INCLUDED


#include <stdbool.h>
#include "constants.h"
#include "config.h"
/** opaque omniGlass structure used by all user-facing library calls */
struct omniglass;

/** make sure to call this initializer before anything else
 @param[in] handle a pointer to a pointer of an omniglass handle.
 */
omniglass_operation_results omniglass_init(struct omniglass **handle);

/** this function must be called at ~100hz in the application's main loop in non-blocking mode,
 * or as soon as possible in a separate thread.
 @param[in] handle the handle to omniglass
 */
int omniglass_step(struct omniglass *handle);

/** representation of a potential contact point between a finger and the touchpad.
 */
typedef struct omniglass_raw_touchpoint {
  bool is_touching; /**< for this point, there is a finger contacting the surface. */
  double x; /**< x coordinate (drag units, starting from 0, from left to right ) */
  double y; /**< y coordinate (drag units, starting from 0, from  up to down) */
} omniglass_raw_touchpoint;

/** a report of contact points, barely filtered from the latest input events acquired from the platform */
typedef struct omniglass_raw_report {
  int points_max; /**< number of touch points on the "points" array. note: this value means "up to this many may touch at once", do not mistake it for "there are this many touching right now"*/
  omniglass_raw_touchpoint *points; /**< array with all the touch points that may be detected by the touchpad. */
} omniglass_raw_report;

/** get the address of the report for raw touch points 
 * get the address of the memory region where the touch report from the active touchpad is located.
 * @param[in] handle the handle to omniglass
 * @param[out] report (output) after executing this function, the content of report will be a pointer to the raw multitouch report of the active touchpad.
 */
omniglass_operation_results omniglass_get_raw_report(struct omniglass *handle, omniglass_raw_report **report);

/** a representation of the touchpad's capabilities relevant to gesture detection.
 */
typedef struct omniglass_raw_specifications {
  double width;
  double height;
  int max_points;
} omniglass_raw_specifications;

/** get address of touchpad specifications.
 * get the address of the memory region where the width, height and other parameters from the active touchpad are defined.
 * @param[in] handle the handle to omniglass.
 * @param[out] specs (output) after executing this function the content of specs will be a pointer to the raw specifications of the active touchpad.
 */
omniglass_operation_results omniglass_get_touchpad_specifications(struct omniglass *handle, omniglass_raw_specifications **specs);

/* CALLBACK SECTION (listen, remove listener, define gestures)
 *
 * most callbacks accept a "passthrough" void pointer that the user can provide at register-time.
 * this allows passing to each callback the data it needs without using global variables.
*/
typedef void (*omniglass_callback_touches_changed)(omniglass_raw_report*, void*);
typedef void (*omniglass_callback_slide)(double, double, void*);
typedef void (*omniglass_callback_pressed)(int);
typedef void (*omniglass_callback_released)(int);
typedef void (*omniglass_callback_edge)(double, void*);

int omniglass_register_callback(struct omniglass *handle, void (*callback) (), omniglass_touchpad_edge edge);

/** set a callback function for points touched.
 * if the number of appendages contacting the surface changes, this callback is triggered.
 * the callback function is called.
 * @param[in] handle the omniglass handle.
 * @param[in] callback the callback for the event.
 * @param[in,out] passthrough whatever pointer is placed here will be forwarded unchanged to the callback.
 */
omniglass_operation_results omniglass_listen_gesture_touches_changed(struct omniglass *handle, omniglass_callback_touches_changed callback, void *passthrough);

/** set a callback function for finger slide.
 * whenever at least one finger has been in contact accross the surface and its contact position has moved since the last call to omniglass_step(), this callback is triggered.
 * @param[in] handle the handle to omniGlass.
 * @param[in,out] callback the function that will be called when a slide has happen
 */
omniglass_gesture_operation_result omniglass_listen_gesture_slide(struct omniglass *handle, omniglass_callback_slide callback, void *passthrough);

/** remove the listener for touch slide gestures.
 *  @param[in] handle a handle to omniglass.
 * */
void omniglass_disable_gesture_slide(struct omniglass *handle);

/** schedule a function to be called whenever fingers have dragged accross a certain edge.
 * @param[in] handle the omniglass handle.
 * @param[in] callback this function is called upon edge scroll
 * @param[in] edge one of the touchpad's 4 edges where the dragging will be scanned for
 * @param[in,out] passthrough this arbitrary data is forwarded to the callback function.
 */
omniglass_gesture_operation_result omniglass_listen_gesture_edge(struct omniglass *handle, omniglass_callback_edge callback, omniglass_touchpad_edge edge, void *passthrough);

/** disable all edge scroll detection callbacks.
 * @param[in] handle the omniglass handle.
 */
void omniglass_disable_gesture_edge(struct omniglass *handle);

omniglass_gesture_operation_result omniglass_listen_gesture_edge_left(struct omniglass *handle, omniglass_callback_edge callback, void * passthrough);
omniglass_gesture_operation_result omniglass_listen_gesture_edge_right(struct omniglass *handle, omniglass_callback_edge callback, void *passthrough);
omniglass_gesture_operation_result omniglass_listen_gesture_edge_top(struct omniglass *handle, omniglass_callback_edge callback, void *passthrough);
omniglass_gesture_operation_result omniglass_listen_gesture_edge_bottom(struct omniglass *handle, omniglass_callback_edge callback, void *passthrough);

omniglass_gesture_operation_result omniglass_enable_parameter_swipe(struct omniglass *handle, double *swipe_x, double *swipe_y);

#endif
