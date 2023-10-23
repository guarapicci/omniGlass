/*
OmniGlass: touchpad gesture detection engine.
Copyright (C) 2023 Felipe Choi

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
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
    DISTANCE_TO_SURFACE /** (note: only Galaxy S5 mini digitizers are known to have this feature) distance between object extremity and the sensitive surface, in millimters accross the surface normal.*/
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
 * this data structure contains position data associated to each point.
 NOTE: Exact number of touches on every multitouch report is per-session, computed at initialization time.
 */
typedef struct multitouch_report{
    //ordered array of touch points
    //(each index identifies a single finger)
    touch_point *touches;
    
    //ordered array of touch parameter arrays
    extended_touch_parameter **extended_touch_parameters;
} multitouch_report;

int multitouch_next_report(struct multitouch_report *report);

typedef struct touchpad_params{
    int min_x;
    int max_x;
    int min_y;
    int max_y;
    int touch_count;
    int extended_touch_parameter_count;
} touchpad_params;

/** opaque omniGlass structure used by all library calls */
struct omniglass;

/**make sure to call this initializer before anything else*/
int omniglass_init(omniglass **handle);

/**this function must be called at ~100hz in the application's main loop in non-blocking mode,
 * or as soon as possible in a separate thread.*/
int omniglass_step(omniglass *handle);

int omniglass_register_callback(void (*callback) ());

