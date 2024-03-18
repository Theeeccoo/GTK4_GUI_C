#include <assert.h>
#include <stdlib.h>


#include "line.h"

struct line
{
    int      desired_algh;    /** << Desired algorithm to draw. */
    int      l_id;            /** << Line identifier.           */
    int      was_clipped;     /** << If line was clipped.       */

    point_tt initial;         /** << Initial point.             */
    point_tt final;           /** << Final point.               */

    point_tt clipped_initial; /** << clipped initial point.     */
    point_tt clipped_final;   /** << clipped final point.       */
};

/**
 * @brief Next available line identification number.
*/
static int next_l_id = 0;

/**
 * @brief Initializes the Line structure.
 * 
 * @param initial Initial point of a line.
 * @param final   Final point of a line.
 * @param algh    Desired algorithm to draw given line.
 * 
 * @returns A line.
*/
line_tt line_create(struct point *initial, struct point *final, int algh)
{
    struct line *l = (struct line*) malloc(sizeof(struct line));
    l->desired_algh = algh;
    l->was_clipped = 0;
    l->l_id = next_l_id++;
    l->initial = initial;
    l->final = final;
    l->clipped_initial = NULL;
    l->clipped_final = NULL;

    return (l);
}

/**
 * @brief Destroys the Line structure.
 * 
 * @param l Given line.
*/
void line_destroy(struct line *l)
{
    /* Sanity Check. */
    assert( l != NULL );
    if ( l->was_clipped )
    {
        point_destroy(l->clipped_initial);
        point_destroy(l->clipped_final);
    }
    point_destroy(l->initial);
    point_destroy(l->final);

    free(l);
}

/**
 * @brief Adds new points to a line whenever it gets clipped. If points are NULL, it implies that line won't be clipped anymore.
 * 
 * @param l       Given line.
 * @param initial Initial clipped point of a line.
 * @param final   Final clipped point of a line.
 * @param flag    Defines which type of clipping ocurred. 0 = None, 1 = Clipped, 2 = Clipped but not drawn
*/
void line_add_clipped_points(struct line *l, struct point *c_i, struct point *c_f, int flag)
{
    /* Sanity Check. */
    assert( l != NULL );

    l->was_clipped = flag;
    l->clipped_initial = c_i;
    l->clipped_final = c_f;
    
}

/**
 * @brief Returns Line's original points.
 * 
 * @param l Desired Line.
 * 
 * @returns Line's original points. Initial = 1st index, Final = 2nd index.
*/
point_tt* line_get_points(const struct line *l)
{
    /* Sanity Check. */
    assert( l != NULL );
    struct point **points = (point_tt*) malloc(sizeof(point_tt) * 2);
    points[0] = l->initial;
    points[1] = l->final;

    return (points);
}

/**
 * @brief Returns Line's clipped points.
 * 
 * @param l Desired Line.
 * 
 * @returns Line's clipped points. Initial = 1st index, Final = 2nd index.
*/
point_tt* line_get_clipped_points(const struct line *l)
{
    /* Sanity Check. */
    assert( l != NULL );
    struct point **points = (point_tt*) malloc(sizeof(point_tt) * 2);
    points[0] = l->clipped_initial;
    points[1] = l->clipped_final;

    return (points);
}

/**
 * @brief Returns Line's drawing algh.
 * 
 * @param l Desired Line.
 * 
 * @returns Line's drawing algh. 1 = DDA, 2 = Bresenham.
*/
int line_get_algh(const struct line *l)
{
    /* Sanity Check. */
    assert( l != NULL );

    return (l->desired_algh);
}

/**
 * @brief Checks if line was clipped.
 * 
 * @param l Desired Line.
 * 
 * @returns If line was clipped.
*/
int line_was_clipped(const struct line *l)
{
    /* Sanity Check. */
    assert( l != NULL );

    return (l->was_clipped);
}

/**
 * @brief Returns line's id.
 * 
 * @param l Given line.
 * 
 * @returns Returns line's id.
*/
int line_id(const struct line *l)
{
    /* Sanity Check. */
    assert( l != NULL );

    return (l->l_id);
}