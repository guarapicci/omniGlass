

/** opaque omniGlass structure used by all user-facing library calls */
struct omniglass;

/**make sure to call this initializer before anything else*/
int omniglass_init(struct omniglass **handle);

/**this function must be called at ~100hz in the application's main loop in non-blocking mode,
 * or as soon as possible in a separate thread.*/
int omniglass_step(struct omniglass *handle);

int omniglass_register_callback(void (*callback) ());
