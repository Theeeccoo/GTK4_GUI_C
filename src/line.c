#include <assert.h>
#include <stdlib.h>


#include "line.h"

struct line
{
    int      desired_algh;    /** Desired algorithm to draw.  */

    int      l_id;            /** << Line identifier.         */
    point_tt initial;         /** << Initial point.           */
    point_tt final;           /** << Final point.             */

    int      was_cropped;     /** << If line was cropped.     */
    point_tt cropped_initial; /** << Cropped initial point.   */
    point_tt cropped_final;   /** << Cropped final point.     */
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
 * 
 * @returns A line.
*/
line_tt line_create(struct point *initial, struct point *final, int algh)
{
    struct line *l = (struct line*) malloc(sizeof(struct line));
    l->desired_algh = algh;
    l->was_cropped = 0;
    l->l_id = next_l_id++;
    l->initial = initial;
    l->final = final;
    l->cropped_initial = NULL;
    l->cropped_final = NULL;

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
    if ( l->was_cropped )
    {
        point_destroy(l->cropped_initial);
        point_destroy(l->cropped_final);
    }
    l->was_cropped = 0;
    point_destroy(l->initial);
    point_destroy(l->final);


    free(l);
}

void line_add_cropped_points(struct line *l, struct point *c_i, struct point *c_f)
{
    /* Sanity Check. */
    assert( l != NULL );

    l->was_cropped = 1;
    l->cropped_initial = c_i;
    l->cropped_final = c_f;
}

/**
 * @brief Returns line id.
 * 
 * @param l Given line.
 * 
 * @returns Returns line id.
*/
int line_id(const struct line *l)
{
    /* Sanity Check. */
    assert( l != NULL );

    return (l->l_id);
}