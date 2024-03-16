#include <assert.h>
#include <stdlib.h>


#include "point.h"
#include "color.h"

struct point
{
    int      p_id;    /** << Point identifier.                        */
    int      taken;   /** << If point is already taken by any object. */
    double   coord_x; /** << X coordinate of drawn point.             */
    double   coord_y; /** << Y coordinate of drawn point.             */
    color_tt pcolor;  /** << Desired color for given Point.           */  
};

/**
 * @brief Next available point identification number.
*/
static int next_p_id = 0;

/**
 * @brief Initializes the Point structure.
 * 
 * @param x X coordinate.
 * @param y Y coordinate.
 * 
 * @returns A point.
*/
point_tt point_create(double x,
                      double y)
{
    struct point *p = (struct point*) malloc(sizeof(struct point));
    p->p_id = next_p_id++;
    p->taken = 0;
    p->coord_x = x;
    p->coord_y = y;
    p->pcolor = NULL;

    return (p);
}

/**
 * @brief Destroy given Point.
 * 
 * @param p Given point.
*/
void point_destroy(struct point *p)
{
    /* Sanity Check. */
    assert( p != NULL );

    color_destroy(p->pcolor);
    free(p);
}

/**
 * @brief Returns X coordinate of a given point.
 * 
 * @param p Given point.
 * 
 * @returns X coordinate of given point.
*/
double point_x_coord(const struct point *p)
{
    /* Sanity Check. */
    assert( p != NULL );

    return (p->coord_x);
}

/**
 * @brief Returns Y coordinate of a given point.
 * 
 * @param p Given point.
 * 
 * @returns Y coordinate of given point.
*/
double point_y_coord(const struct point *p)
{
    /* Sanity Check. */
    assert( p != NULL );


    return (p->coord_y);
}

/**
 * @brief Sets a new coordinate to a given point.
 * 
 * @param p Given point.
 * @param x X coordinate.
 * @param y Y coordinate. 
*/
void point_set_coord(struct point *p,
                     double        x, 
                     double        y)
{
    /* Sanity Check. */
    assert( p != NULL );

    p->coord_x = x;
    p->coord_y = y;
}

/**
 * @brief Returns Point id.
 * 
 * @param p Given point.
 * 
 * @returns Returns Point id.
*/
int point_id(const struct point *p)
{
    /* Sanity Check. */
    assert( p != NULL );

    return (p->p_id);
}

/**
 * @brief Sets a point as taken by an object.
 * 
 * @param p Given point.
*/
void point_take(struct point *p)
{
    /* Sanity Check. */
    assert( p != NULL );

    p->taken = 1;
}

/**
 * @brief Returns if given point is already taken by any object.
 * 
 * @param p Given point.
 * 
 * @returns If given point is already taken. 0 = No, 1 = Yes.
*/
int point_is_taken(const struct point *p)
{
    /* Sanity Check. */
    assert( p != NULL );

    return (p->taken);
}

/**
 * @brief Defines which color a point should have (RGB-based).
 * 
 * @param p Specified point.
 * @param r RED.
 * @param g GREEN.
 * @param b BLUE.
*/
void point_define_color(struct point *p,
                        double        r,
                        double        g,
                        double        b)
{
    /* Sanity Check. */
    assert( p != NULL );
    p->pcolor = color_create(r, g, b);
}

/**
 * @brief Returns the pre-defined color from a defined point.
 * 
 * @param p Given point.
 * 
 * @returns Returns the color from a point.
*/
color_tt point_color(const struct point *p)
{
    /* Sanity Check. */
    assert( p != NULL );
    assert( p->pcolor != NULL );

    return (p->pcolor);
}