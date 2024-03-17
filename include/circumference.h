#ifndef CIRCUMFERENCE_H_
#define CIRCUMFERENCE_H_

    #include "point.h"

    /**
     * @brief Pointer to a circumference struct.
    */
    typedef struct circumference * circumference_tt;

    /**
     * @brief Pointer to const a circumference struct.
    */
    typedef const struct circumference * const_circumference_tt;

    /**
     * @brief Operations on Circumference.
    */
    /**@(*/
    extern circumference_tt circumference_create(point_tt, point_tt);
    extern void             circumference_destroy(circumference_tt);
    extern int              circumference_id(const_circumference_tt);
    extern double           circumference_radius(const_circumference_tt);

    extern point_tt*        circumference_get_points(const_circumference_tt);
    /**@)*/

#endif /* CIRCUMFERENCE_H_ */