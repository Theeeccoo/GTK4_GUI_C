#ifndef POLYGON_H_
#define POLYGON_H_


    #include "point.h"
    #include "array.h"
    #define MAX_POINTS 50

    /**
     * @brief Pointer to a polygon struct.
    */
   typedef struct polygon * polygon_tt;

    /**
     * @brief Pointer to a const polygon struct.
    */
   typedef const struct polygon * const_polygon_tt;

    /**
     * @brief Operations on polygons.
    */
    /**@(*/
    extern polygon_tt polygon_create(point_tt*, int, int);
    extern void       polygon_destroy(polygon_tt);
    extern int        polygon_id(const_polygon_tt);
    extern void       polygon_add_clipped_points(polygon_tt, point_tt*, int, int);
    extern int        polygon_get_algh(const_polygon_tt);
    extern int        polygon_was_clipped(const_polygon_tt);

    extern array_tt  polygon_get_points(const_polygon_tt);
    extern array_tt  polygon_get_clipped_points(const_polygon_tt);
    /**@)*/

#endif /* POLYGON_H_ */