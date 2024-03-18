#include <assert.h>
#include <stdlib.h>

#include "polygon.h"

struct polygon
{
    int      desired_algh;   /** << Desired algorithm to draw. */
    int      pl_id;          /** << Polygon identifier.        */
    int      was_clipped;    /** << If polygon was clipped.    */


    array_tt points;         /** << Polygon's points.          */
    array_tt clipped_points; /** << Polygon's croppped points. */
};

/**
 * @brief Next available polygon identification number.
*/
static int next_pl_id = 0;

/**
 * @brief Initializes the Polygon structure.
 * 
 * @param points Array that contains all points of given Polygon.
 * @param size   Number of points.
 * @param Algh   Drawing algorithm desired. 
 * 
 * @returns A Polygon.
*/
polygon_tt polygon_create(struct point **points, int size, int algh)
{
    struct polygon *pl = (struct polygon*) malloc(sizeof(struct polygon));

    assert( size < MAX_POINTS );
    
    pl->desired_algh = algh;
    pl->pl_id = next_pl_id++;
    pl->was_clipped = 0;
    pl->points = array_create(MAX_POINTS);
    pl->clipped_points = array_create(MAX_POINTS);
    

    for ( int i = 0; i < size; i++ )
    {
        array_set(pl->points, i, points[i]);
    }

    return (pl);
}

/**
 * @brief Adds new points to a polygon whenever it gets clipped. If size == 0, it imples that polygon won't be clipped anymore
 * 
 * @param pl     Given polygon.
 * @param size   Number of clipped points.
 * @param points Clipped points of a polygon.
 * @param flag    Defines which type of clipping ocurred. 0 = None, 1 = Clipped, 2 = Clipped but not drawn
*/
void polygon_add_clipped_points(struct polygon *pl, struct point **points, int size, int flag)
{
    /* Sanity Check. */
    assert( pl != NULL );
    assert( size < MAX_POINTS );
    
    pl->was_clipped = flag;

    if ( pl->was_clipped == 1 )
    {
        for ( int i = 0; i < size ; i++ )
        {
            array_set(pl->clipped_points, i + (array_get_curr_num(pl->clipped_points) - 1), points[i++]);
        }
    } 
     
}

/**
 * @brief Returns Polygon's original points.
 * 
 * @param pl Desired Polygon.
 * 
 * @returns Polygon's original points.
*/
array_tt polygon_get_points(const struct polygon *pl)
{
    /* Sanity Check. */
    assert( pl != NULL );
    
    return (pl->points);
}

/**
 * @brief Returns Polygons's clipped points.
 * 
 * @param pl Desired Polygons.
 * 
 * @returns Polygons's clipped points.
*/
array_tt polygon_get_clipped_points(const struct polygon *pl)
{
    /* Sanity Check. */
    assert( pl != NULL );
    
    return (pl->clipped_points);
}


/**
 * @brief Destroys the Polygon structure.
 * 
 * @param pl Given polygon.
*/
void polygon_destroy(struct polygon *pl)
{
    /* Sanity Check. */
    assert( pl != NULL );
    array_destroy(pl->clipped_points);
    array_destroy(pl->points);

    free(pl);
}

/**
 * @brief Returns Polygon's drawing algh.
 * 
 * @param pl Desired Polygon.
 * 
 * @returns Polygon's drawing algh. 1 = DDA, 2 = Bresenham.
*/
int polygon_get_algh(const struct polygon *pl)
{
    /* Sanity Check. */
    assert( pl != NULL );

    return (pl->desired_algh);
}

/**
 * @brief Checks if Polygon was clipped.
 * 
 * @param pl Desired Polygon.
 * 
 * @returns If Polygon was clipped.
*/
int polygon_was_clipped(const struct polygon *pl)
{
    /* Sanity Check. */
    assert( pl != NULL );

    return (pl->was_clipped);
}

/**
 * @brief Returns polygon's id.
 * 
 * @param pl Given polygon.
 * 
 * @returns Returns polygon's id.
*/
int polygon_id(const struct polygon *pl)
{
    /* Sanity Check. */
    assert( pl != NULL );

    return (pl->pl_id);
}

