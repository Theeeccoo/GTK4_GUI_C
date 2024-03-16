#include <assert.h>
#include <stdlib.h>

#include "polygon.h"

struct polygon
{
    int      desired_algh;   /** << Desired algorithm to draw. */
    int      pl_id;          /** << Polygon identifier.        */
    int      was_cropped;    /** << If polygon was cropped.    */


    array_tt points;         /** << Polygon's points.          */
    array_tt cropped_points; /** << Polygon's croppped points. */
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
    pl->was_cropped = 0;
    pl->points = array_create(MAX_POINTS);
    pl->cropped_points = array_create(MAX_POINTS);
    

    for ( int i = 0; i < size; i++ )
    {
        array_set(pl->points, i, points[i]);
    }

    return (pl);
}

/**
 * @brief Adds new points to a polygon whenever it gets cropped.
 * 
 * @param l       Given polygon.
 * @param initial Initial cropped point of a polygon.
 * @param final   Final cropped point of a polygon.
*/
void polygon_add_cropped_points(struct polygon *pl, struct point **points)
{
    /* Sanity Check. */
    assert( pl != NULL );
    assert( points != NULL );
    int size = sizeof(points) / sizeof(points[0]);
    assert( size < MAX_POINTS );
    
    pl->was_cropped = 1;
    for ( int i = 0; i < size; i++ )
    {
        array_set(pl->cropped_points, i, points[i]);
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
 * @brief Returns Polygons's cropped points.
 * 
 * @param l Desired Polygons.
 * 
 * @returns Polygons's cropped points.
*/
array_tt polygons_get_cropped_points(const struct polygon *pl)
{
    /* Sanity Check. */
    assert( pl != NULL );
    
    return (pl->cropped_points);
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
    array_destroy(pl->cropped_points);
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

