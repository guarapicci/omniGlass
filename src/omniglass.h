
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

int omniglass_register_callback(void (*callback) ());
