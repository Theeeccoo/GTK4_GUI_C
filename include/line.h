#ifndef LINE_H_
#define LINE_H_

    #include "point.h"

    /**
     * @brief Pointer to a line struct.
    */
   typedef struct line * line_tt;

    /**
     * @brief Pointer to a const line struct.
    */
    typedef const struct line * const_line_tt;

    /**
     * @brief Operations on Line.
    */
    /**@(*/
    extern line_tt line_create(point_tt, point_tt, int);
    extern void    line_destroy(line_tt);
    extern int     line_id(const_line_tt);
    extern void    line_add_cropped_points(line_tt, point_tt, point_tt);
    extern int     line_get_algh(const_line_tt);
    /**@)*/

#endif /* LINE_H_ */