#include <assert.h>
#include <stdlib.h>

#include "clipping.h"

struct clip
{
    int desired_algh; /** << Desired algorithm to draw. */
    int cl_id;        /** << Clip identifier.           */

    array_tt points;  /** << Clip's points.             */
};

/**
 * @brief Next available clip identification number.
*/
static int next_cl_id = 0;

/**
 * @brief Initializes the Clip structure.
 * 
 * @param points Array that contains all points of given Clip.
 * @param size   Number of points.
 * @param Algh   Drawing algorithm desired. 
 * 
 * @returns A Clip structure.
*/
clip_tt clip_create(struct point **points, int size, int algh)
{
    struct clip *cl = (struct clip*) malloc(sizeof(struct clip));

    assert( size == NUM_CLIP_POINTS );

    cl->desired_algh = algh;
    cl->cl_id = next_cl_id++;
    cl->points = array_create(NUM_CLIP_POINTS);

    for ( int i = 0; i < size; i++ )
    {
        array_set(cl->points, i, points[i]);
    }

    return cl;
}

/**
 * @brief Returns Clip's points.
 * 
 * @param cl Desired Clip.
 * 
 * @returns Clip's points.
*/
array_tt clip_get_points(const struct clip *cl)
{
    /* Sanity Check. */
    assert( cl != NULL );
    
    return (cl->points);
}

/**
 * @brief Destroys the Clip structure.
 * 
 * @param cl Given clip.
*/
void clip_destroy(struct clip *cl)
{
    /* Sanity Check. */
    assert( cl != NULL );
    array_destroy(cl->points);

    free(cl);
}

/**
 * @brief Calculate and returns the xmin xmax ymin ymax of clip based on its points.
 * 
 * @param cl Given clip.
 * 
 * @returns The xmin xmax ymin ymax of clip based on its points. 1st idx = xmin, 2nd idx = xmax, 3rd idx = ymin, 4th idx = ymax.
*/
double* clip_get_maxmin(const struct clip *cl)
{
    /* Sanity Check. */
    assert( cl != NULL );
    double *result = (double*) malloc(sizeof(double) * 4);
    double x_first = point_x_coord(array_get(cl->points, 0)),
           x_third = point_x_coord(array_get(cl->points, 2)),
           y_first = point_y_coord(array_get(cl->points, 0)),
           y_third = point_y_coord(array_get(cl->points, 2));

    if ( x_first < x_third) 
    {
        result[0] = point_x_coord(array_get(cl->points, 0));
        result[1] = point_x_coord(array_get(cl->points, 2));
    }
    else
    { 
        result[0] = point_x_coord(array_get(cl->points, 2));
        result[1] = point_x_coord(array_get(cl->points, 0));
    }

    if ( y_first < y_third )        
    {
        result[2] = point_y_coord(array_get(cl->points, 0));
        result[3] = point_y_coord(array_get(cl->points, 2));
    }
    else
    { 
        result[2] = point_y_coord(array_get(cl->points, 2));
        result[3] = point_y_coord(array_get(cl->points, 0));
    }
    

    return result;
}

/**
 * @brief Returns clip's id.
 * 
 * @param cl Given clip.
 * 
 * @returns Returns clip's id.
*/
int clip_id(const struct clip *cl)
{
    /* Sanity Check. */
    assert( cl != NULL );

    return (cl->cl_id);
}