
#include <stdbool.h>
/**
 * Touch point in left-to-right x and bottom-to-top y.
 * Touch points include a touch ID;
 */
typedef struct touch_point{
    int x;
    int y;
    bool touched;
} touch_point;

/**
 * type identification for extended parameters of a touch contact
 * (might eventually include crazy stuff like material type, distance to actual surface, etc)
 * @
 */
typedef enum touchParameter {
    PRESSURE, /**< pressure (unknown metric) */
    AREA,   /**< area of the touch being tracked, in cm^2 */
    DISTANCE_TO_SURFACE /** (note: only Galaxy S5 mini digitizers are known to have this feature) distance between object extremity and the sensitive surface, in millimiters accross the surface normal.*/
} touch_parameter;

//extended parameters (many touchpad drivers may not support any of these.)
typedef unsigned int touch_pressure;    //pressure applied on a contact point.
typedef unsigned int touch_area;        //combined area of all the intersection belonging to an identified contact region centered around a point.
typedef unsigned int touch_distance_to_surface;  //distance in millimeters;

/**an extended parameter associated with a touch point.
 * parameters must be associated to their parent points.
 * raw data associated to a parameter must be casted to its corresponding type.
 */
typedef struct extended_touch_parameter{
    touch_parameter type;   /**< the type of a touch parameter. Use this to identify the data type and to select a proper casting for the raw data. */
    void *data;             /**< the raw data of the parameter. Cast this to "touch_" suffixed by the enumerator's name lowercased */
} extended_touch_parameter;

/** central data structure for touch tracking.
 * this data structure contains position data associated to each point*/
typedef struct multitouch_report{
    //ordered array or touch points
    //(each index identifies a single finger)
    touch_point *touches;
    
    //ordered array of touch parameter arrays
    extended_touch_parameter **extended_touch_parameters;
} multitouch_report;



