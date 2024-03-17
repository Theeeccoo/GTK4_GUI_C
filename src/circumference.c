#include <assert.h>
#include <stdlib.h>
#include <math.h>

#include "circumference.h"

struct circumference
{
    int c_id;        /** << Circumference identifier.     */

    point_tt center; /** << Circumference's center point. */
    point_tt border; /** << Circumference's border point. */
};

/**
 * @brief Next available circumference identification number.
*/
static int next_c_id = 0;


/**
 * @brief Initializes the Line structure.
 * 
 * @param center Center point of a line.
 * @param border Border point of a line.
 *
 * @returns A Circumference.
*/
circumference_tt circumference_create(struct point *center, struct point *border)
{
    struct circumference *c = (struct circumference*) malloc(sizeof(struct circumference));
    c->c_id = next_c_id++;
    c->center = center;
    c->border = border;

    return (c);
}

/**
 * @brief Destroys the Circumference structure.
 * 
 * @param c Given circumference.
*/
void circumference_destroy(struct circumference *c)
{
    /* Sanity Check. */
    assert( c != NULL );
    
    point_destroy(c->center);
    point_destroy(c->border);

    free(c);
}

/**
 * @brief Returns Circumference's original points.
 * 
 * @param c Desired Circumference.
 * 
 * @returns Circumference's points. Center = 1st index, Border = 2nd index.
*/
point_tt* circumference_get_points(const struct circumference *c)
{
    /* Sanity Check. */
    assert( c != NULL );
    struct point **points = (point_tt*) malloc(sizeof(point_tt) * 2);
    points[0] = c->center;
    points[1] = c->border;

    return (points);
}

/**
 * @brief Returns Circumference's radius. Distance between border point and center point.
 * 
 * @param c Given Circumference.
 * 
 * @returns Circumference's radius.
*/
double circumference_radius(const struct circumference *c)
{
    /* Sanity Check. */
    assert( c != NULL );
    return sqrt(pow(point_x_coord(c->border) - point_x_coord(c->center), 2) + pow(point_y_coord(c->border) - point_y_coord(c->center), 2));
}

/**
 * @brief Returns circumference's id.
 * 
 * @param c Given circumference.
 * 
 * @returns Returns circumference's id.
*/
int circumference_id(const struct circumference *c)
{
    /* Sanity Check. */
    assert( c != NULL );

    return (c->c_id);
}