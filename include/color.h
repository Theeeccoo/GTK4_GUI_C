#ifndef COLOR_H_
#define COLOR_H_

    /**
     * @brief Pointer to a color struct.
    */
   typedef struct color * color_tt;

    /**
     * @brief Operations on Line.
    */
    /**@(*/
    extern color_tt color_create(double, double, double);
    extern void     color_destroy(color_tt);
    extern double*  color_get_colors(color_tt);
    /**@)*/

#endif /* COLOR_H_ */