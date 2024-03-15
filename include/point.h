#ifndef POINT_H_
#define POINT_H_

    #include "color.h"

    /**
     * @brief Pointer to a point struct.
    */
    typedef struct point * point_tt;

    /**
     * @brief Pointer to a const point struct.
    */
    typedef const struct point * const_point_tt;

    /**
     * @brief Operations on Point.
    */
    /**@(*/
    extern point_tt point_create(double, double);
    extern void     point_destroy(point_tt);
    extern double   point_x_coord(const_point_tt);
    extern double   point_y_coord(const_point_tt);
    extern void     point_set_coord(point_tt, double, double);
    extern int      point_id(const_point_tt);

    extern void     point_define_color(point_tt, double, double, double);
    extern color_tt point_color(const_point_tt);
    /**@)*/

#endif /* POINT_H_ */