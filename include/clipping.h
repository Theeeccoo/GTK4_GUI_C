#ifndef CLIPPING_H_
#define CLIPPING_H_

    #include "array.h"
    #include "point.h"
    #define  NUM_CLIP_POINTS 4

    /**
     * @brief Pointer to a clip struct.
    */
    typedef struct clip * clip_tt;

    /**
     * @brief Pointer to a const clip struct.
    */
    typedef const struct clip * const_clip_tt;

    /**
     * @brief Operations on Clip.
    */
    /**@(*/
    extern clip_tt  clip_create(point_tt*, int, int);
    extern void     clip_destroy(clip_tt);
    extern int      clip_id(const_clip_tt);

    extern array_tt clip_get_points(const_clip_tt);
    extern double*  clip_get_maxmin(const_clip_tt);
    /**@)*/


#endif /* CLIPPING_H_ */