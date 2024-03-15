#include <assert.h>
#include <stdlib.h>


#include "color.h"

struct color
{
    double red_value;   /** << Red value.   */
    double green_value; /** << Green value. */
    double blue_value;  /** << Blue value.  */
};

/**
 * @brief Initializes the Color structure.
 * 
 * @param r Red value.
 * @param g Green value.
 * @param b Blue value.
 * 
 * @returns A color.
*/
color_tt color_create(double r, 
                      double g, 
                      double b)
{
    struct color *c = (struct color*) malloc(sizeof(struct color));

    c->red_value = r;
    c->green_value = g;
    c->blue_value = b;

    return (c);
}

/**
 * @brief Destroy given Color.
 * 
 * @param c Given color.
*/
void color_destroy(struct color *c)
{
    /* Sanity Check. */
    assert( c != NULL );
    free(c);
}

/**
 * @brief Returns RGB values from current color
 * 
 * @param c Given color.
 * 
 * @returns Array that contains RBG values from color. 1st Position = Red, 2nd Position = Green, 3rd Position = Blue.
*/
double* color_get_colors(struct color *c)
{
    double *rgb = (double*) malloc(sizeof(double) * 3);

    rgb[0] = c->red_value;
    rgb[1] = c->green_value;
    rgb[2] = c->blue_value;

    return rgb;
}
