#include <assert.h>
#include <cairo.h>
#include <gtk/gtk.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// Used to make the app fit correctly, proportionately, into users window
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>

#include "array.h"
#include "line.h"
#include "point.h"
#include "polygon.h"
#include "circumference.h"
#include "clipping.h"

// #define MAX_POINTS 10

static cairo_surface_t *surface = NULL;
static int algh = 0;

/**
 * @brief All Widgets from our interface.
*/
struct {
    GtkWidget      *window, 
                   *frame,
                   *drawing_area, 
                   *main_box,
                   *frame_box,
                   *option_box,
                   *dropdown_transformations,
                   *dropdown_algorithms,
                   *dropdown_drawings,
                   *dropdown_croppings,
                   *main_input,
                   *label;
    GtkGesture     *drag, 
                   *press;
    GtkEntryBuffer *entry_buffer;
} Widgets;


/**
 * @brief Current clicked Points.
*/
static array_tt arr_points;

/**
 * @brief Current drawn Lines.
*/
static array_tt arr_lines;

/**
 * @brief Current drawn Polygons.
*/
static array_tt arr_polygons;

/**
 * @brief Current drawn Circumferences.
*/
static array_tt arr_circumferences;

/**
 * @brief Current drawn Clips.
*/
static array_tt arr_clips;

/**
 * @brief Removes all points drawn in canvas.
*/
static void clear_surface(int flag)
{
    cairo_t *cr = cairo_create(surface);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    cairo_destroy(cr);
    gtk_label_set_label(GTK_LABEL(Widgets.label), "Debug informations here...");

    if ( flag ) 
    {
        // Recreating array that contains all points
        array_destroy(arr_points);
        arr_points = array_create(MAX_POINTS);
        array_destroy(arr_lines);
        arr_lines = array_create(MAX_POINTS);
        array_destroy(arr_polygons);
        arr_polygons = array_create(MAX_POINTS);
        array_destroy(arr_circumferences);
        arr_circumferences = array_create(MAX_POINTS);
        array_destroy(arr_clips);
        arr_clips = array_create(MAX_POINTS);
    }
}

/**
 * @brief Whenever screen is resized, remove current points to prevent a bug in visualization.
 * 
 * @param widget Drawing area that contains the drawn points
*/
static void resize_cb(GtkWidget *widget,
                      int        width,
                      int        height,
                      gpointer   data)
{
    if ( surface )
    {
        cairo_surface_destroy(surface);
        surface = NULL;
    }

    if ( gtk_native_get_surface(gtk_widget_get_native(widget)) ) 
    {
        surface = gdk_surface_create_similar_surface(gtk_native_get_surface(gtk_widget_get_native(widget)), CAIRO_CONTENT_COLOR, gtk_widget_get_width(widget), gtk_widget_get_height(widget));



        clear_surface(1);
    }
}

/**
 * @brief (CALL_BACK) Function that is linked with Drawing Area that lets you draw in drawing_area
 * 
 * @param cairo Object used to draw points
*/
static void draw_cb(GtkDrawingArea *drawing_area,
                    cairo_t        *cr,
                    int             width, 
                    int             height, 
                    gpointer        data)
{
    cairo_set_source_surface(cr, surface, 0, 0);
    cairo_paint(cr);
}


/**
 * @brief Receives two points in canvas and converts it into a string pattern that will be drawn whenever a point is drawn by user
 * 
 * @param x X coordinate in canvas.
 * @param y Y coordinate in canvas.
 * 
 * @returns String pattern. (X, Y)
*/
char* create_string(double x, 
                    double y)
{
    int size  = 0,
        aux_x = (int)round(x), 
        aux_y = (int)round(y);

    char *char_x = (char*) malloc (sizeof(char*) * 8),
         *char_y = (char*) malloc (sizeof(char*) * 8);


    sprintf(char_x, "%d", aux_x);
    sprintf(char_y, "%d", aux_y);
    char_x[7] = char_y[7] = '\0';

    // Text example: (X, Y)
    size += strlen(char_x) + strlen(char_y) + 8;
    char* result = (char*) malloc(sizeof(char) * size);

    strcat(char_x, ", ");
    strcat(char_x, char_y);
    strcpy(result, "(");
    strcat(result, char_x);
    result[strlen(char_x) + 1] = ')';
    result[strlen(char_x) + 2] = '\0';
    free(char_x);
    free(char_y);
    return result;
}

/**
 * @brief Using Cairo, writes the position of drawn x and y in canvas
*/
void draw_text(GtkWidget *widget,
               cairo_t   *cr,
               point_tt   p)
{
    int center_x = 0,
        center_y = 0; 
    gtk_widget_get_size_request(Widgets.drawing_area, &center_x, &center_y);
    double x = point_x_coord(p),
           y = point_y_coord(p);
    char *result = create_string(x, y);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 12.0);

    cairo_move_to(cr, (x + (center_x/2)) - 34, ((center_y/2) - y ) + 15 );
    cairo_show_text(cr, result);

    gtk_widget_queue_draw(widget);
}

/**
 * @brief Using Cairo, draws a rectangle in given positions with thickness of 6
*/
static void draw_brush(GtkWidget *area,
                       cairo_t   *cr,
                       double     x, 
                       double     y,
                       double    *c)
{
    int center_x = 0,
        center_y = 0; 
    gtk_widget_get_size_request(Widgets.drawing_area, &center_x, &center_y);
    cairo_set_source_rgb(cr, c[0], c[1], c[2]);
    cairo_rectangle(cr, (x + (center_x/2))- 3, ((center_y/2) - y ) - 3, 6, 6);
    cairo_fill(cr);
    gtk_widget_queue_draw(area);
}

/**
 * @brief (CALL_BACK) Function called whenever user clicks into canvas
 * 
 * @param x    X coordinate in canvas.
 * @param y    Y coordinate in canvas.
 * @param area Drawing area that point will be added
*/
static void draw(GtkGestureDrag *gesture,
                 double          x,
                 double          y,
                 GtkWidget      *area)
{
    if ( array_get_curr_num(arr_points) == MAX_POINTS )
    {
        gtk_label_set_label(GTK_LABEL(Widgets.label), "WARNING: You have reached maximum amount of Points.");
        return;
    }
    // Redefining center point to be exactly the center of canvas. In canvas 0,0 is in the top-left corner.
    int center_x = 0,
        center_y = 0; 
    gtk_widget_get_size_request(Widgets.drawing_area, &center_x, &center_y);
    x = x - (double)(center_x / 2);
    y = (double)(center_y / 2) - y;
    struct point *p = point_create(x, y);
    array_set(arr_points, array_get_curr_num(arr_points), p);

    point_define_color(p, 0.0, 0.0, 0.0);

    cairo_t *cr;

    cr = cairo_create(surface);
    draw_brush(area, cr, x, y, color_get_colors(point_color(p)));
    draw_text(area, cr, p);
    cairo_destroy(cr);
}

/**
 * @brief Uses bresenham algorithm to draw a line between two points. 
 * 
 * @param pInit  Initial point of Line.
 * @param pFinal Final point of Line.
 * @param area   Drawing area.
*/
void Bresenham(point_tt pInit, 
               point_tt pFinal,
               GtkWidget *area)
{
    cairo_t *cr;
    cr = cairo_create(surface);
    int xi = (int) point_x_coord(pInit),
        yi = (int) point_y_coord(pInit),
        xf = (int) point_x_coord(pFinal),
        yf = (int) point_y_coord(pFinal);

    int dx     = xf - xi,
        dy     = yf - yi,
        x      = xi,
        y      = yi,
        i      = 0,
        p      = 0,
        const1 = 0,
        const2 = 0,
        incrx  = 0,
        incry  = 0;

    if ( dx >= 0 ) incrx = 1;
    else { incrx = -1; dx = (-dx); }

    if ( dy >= 0 ) incry = 1;
    else { incry = -1; dy = (-dy); }

    if ( dy < dx )
    {
        p = 2 * dy - dx;
        const1 = 2 * dy;
        const2 = 2 * (dy - dx);

        for ( int i = 0; i < dx; i++ )
        {
            x += incrx;
            if ( p < 0 ) p += const1;
            else { y += incry; p += const2; }
            draw_brush(area, cr, (double) round(x), (double) round(y), color_get_colors(point_color(pInit)));
        }
    } else 
    {
        p = 2 * dx - dy;
        const1 = 2 * dx;
        const2 = 2 * (dx - dy);

        for ( int i = 0; i < dy; i++ )
        {
            y += incry;
            if ( p < 0 ) p += const1;
            else { x += incrx; p += const2; }
            draw_brush(area, cr, (double) round(x), (double) round(y), color_get_colors(point_color(pInit)));
        }
    }

}

/**
 * @brief Uses DDA algorithm to draw a line between two points. 
 * 
 * @param pInit  Initial point of Line.
 * @param pFinal Final point of Line.
 * @param area   Drawing area.
*/
void DDA(point_tt pInit,
         point_tt pFinal,
         GtkWidget *area)
{ 
    double xi = point_x_coord(pInit),
           yi = point_y_coord(pInit),
           xf = point_x_coord(pFinal),
           yf = point_y_coord(pFinal);

    cairo_t *cr;
    cr = cairo_create(surface);
    double x_var = xf - xi,
           y_var = yf - yi,
           x_incr = 0.0f,
           y_incr = 0.0f,
           x_aux = xi,
           y_aux = yi;
    int    iterations = 0;

    if ( abs(x_var) > abs(y_var) ) iterations = abs(x_var);
    else iterations = abs(y_var);

    x_incr = x_var / iterations;
    y_incr = y_var / iterations;

    for ( int i = 1; i <= iterations; i++ )
    {
        x_aux += x_incr;
        y_aux += y_incr;
        draw_brush(area, cr, (double) round(x_aux), (double) round(y_aux), color_get_colors(point_color(pInit)));
    }
        
    cairo_destroy(cr);
}

/**
 * @brief (CALL_BACK) Function called whenever user wants to clear canvas
 * 
 * @param area Drawing area that will be cleaned
*/
static void clean(GtkGestureClick *gesture,
                  int              n_press,
                  double           x, 
                  double           y,
                  GtkWidget        *area)
{
    clear_surface(1);
    gtk_widget_queue_draw(area);
}

/**
 * @brief (CALL_BACK) Function called whenever user closes main window
*/
static void close_window(void)
{
    array_destroy(arr_points);
    array_destroy(arr_lines);
    array_destroy(arr_polygons);
    array_destroy(arr_circumferences);
    array_destroy(arr_clips);
    if ( surface ) cairo_surface_destroy(surface);
}


/**
 * @brief (CALL_BACK) Function called whenever an option in "Algorithms"drop-down is sellected.
 * Defines which line-drawing (DDA or Bresenham) algorithm should be used to draw.
 * 
 * @param dropdown Dropdown selected
 * @param entry 
*/
static void algorithms_execution(GtkDropDown *dropdown,
                                 gpointer     user_data)
{
    algh = gtk_drop_down_get_selected(dropdown);
}

int number_taken_points()
{
    int num = 0;

    for ( int i = 0; i < array_get_curr_num(arr_points); i++ )
        if ( point_is_taken(array_get(arr_points, i)) ) num++;

    return num;
}

/**
 * @brief Using equality of points, plotting a circumference.
 * 
 * @param area   Drawing area.
 * @param center Circumference's center point.
 * @param x_calc X coordinate to be plotted.
 * @param y_calc Y coordinate to be plotted.
 *
*/
void plot_circle_points(GtkWidget *area,
                       point_tt   center,
                       double     x_calc,
                       double     y_calc)
{
    double x_center = point_x_coord(center),
           y_center = point_y_coord(center);


    cairo_t *cr;
    cr = cairo_create(surface);
    draw_brush(area, cr, (double) (round(x_center) + round(x_calc)), (double) (round(y_center) + round(y_calc)), color_get_colors(point_color(center)));
    draw_brush(area, cr, (double) (round(x_center) - round(x_calc)), (double) (round(y_center) + round(y_calc)), color_get_colors(point_color(center)));
    draw_brush(area, cr, (double) (round(x_center) + round(x_calc)), (double) (round(y_center) - round(y_calc)), color_get_colors(point_color(center)));
    draw_brush(area, cr, (double) (round(x_center) - round(x_calc)), (double) (round(y_center) - round(y_calc)), color_get_colors(point_color(center)));

    draw_brush(area, cr, (double) (round(x_center) + round(y_calc)), (double) (round(y_center) + round(x_calc)), color_get_colors(point_color(center)));
    draw_brush(area, cr, (double) (round(x_center) - round(y_calc)), (double) (round(y_center) + round(x_calc)), color_get_colors(point_color(center)));
    draw_brush(area, cr, (double) (round(x_center) + round(y_calc)), (double) (round(y_center) - round(x_calc)), color_get_colors(point_color(center)));
    draw_brush(area, cr, (double) (round(x_center) - round(y_calc)), (double) (round(y_center) - round(x_calc)), color_get_colors(point_color(center)));

    cairo_destroy(cr);
}

/**
 * @brief Uses the idea of Bresenham's circumference algorithm to identify all points.
 * 
 * @param area Area to draw points.
 * @param c    Given circumference to draw.
*/
void calculate_circumference_points(GtkWidget *area,
                                    struct circumference *c)
{
    int x = 0,
        y = 0, 
        p = 0,
        r = 0;

    r = circumference_radius(c);
    y = r;
    p = 3 - 2 * r;

    struct point *center = circumference_get_points(c)[0];

    plot_circle_points(area, center, x, y);

    while ( x < y )
    {
        if ( p < 0 )
        {
            p = p + 4 * x + 6;
        } else 
        {
            p = p + 4 * (x-y) + 10;
            y--;
        }
        x++;
        plot_circle_points(area, center, x, y);    
    }
}

/**
 * @brief Creates and draws Circumferences with Bresenham's circumference algorithm.
 * 
 * @param area Drawing Area.
 * 
 * @returns True if code was correctly executed, False otherwise.
*/
Bool Circumference(GtkWidget *area)
{

    int controller = number_taken_points(),
        size_p = array_get_curr_num(arr_points);


    if ( (size_p - controller) < 2 ) 
    {
        gtk_label_set_label(GTK_LABEL(Widgets.label), "WARNING: There must be atleast 2 free points to draw a Circumference.");
        return False;
    }

    while( controller < size_p )
    {
        /**
         * Must have atleast 2 points to draw a line,
         * If there is only one, "ignore it".
         */
        if ( (size_p - controller) >= 2 )
        {  
            struct point *pCenter = array_get(arr_points, controller);
            struct point *pBorder = array_get(arr_points, controller + 1);

            if ( point_is_taken(pCenter) ) continue;

            struct circumference *new_circumference = circumference_create(pCenter, pBorder);
            array_set(arr_circumferences, array_get_curr_num(arr_circumferences), new_circumference);
            point_take(pCenter);
            point_take(pBorder);

            calculate_circumference_points(area, new_circumference);
            controller++;
        } 
        controller++;
    }
    return True;
    
}

/**
 * @brief Creates and draws Polygons with previously specified Drawing Algorithm.
 * 
 * @param area Drawing Area.
 * 
 * @returns True if code was correctly executed, False otherwise.
*/
Bool Polygon(GtkWidget *area)
{
    // If no algorithm selected, "Error".
    if ( algh == 0 )
    {
        gtk_label_set_label(GTK_LABEL(Widgets.label), "WARNING: You must provide the drawing algorithm.");
        return False;
    }
    int controller = number_taken_points(),
        size_p = array_get_curr_num(arr_points),
        iterator = 0;

    struct point **points = (point_tt*) malloc(sizeof(point_tt) * (size_p - controller));
    if ( (size_p - controller) >= 3 )
    { 
        // Adding all left points into polygon's structure.
        while ( controller < size_p ) 
        {
            point_tt p = array_get(arr_points, controller++);
            point_take(p);
            points[iterator++] = p;
        }

        struct polygon *polygon = polygon_create(points, iterator, algh);

        array_set(arr_polygons, array_get_curr_num(arr_polygons), polygon);

        for ( int i = 0; i < iterator - 1; i++ )
        {
            struct point *pInit = points[i];
            struct point *pFinal = points[i + 1];

            if ( algh == 1 ) DDA(pInit, pFinal, area);
            else if ( algh == 2 ) Bresenham(pInit, pFinal, area);
        }
        // Closing Polygon
        struct point *pInit = points[iterator - 1];
        struct point *pFinal = points[0];

        if ( algh == 1 ) DDA(pInit, pFinal, area);
        else if ( algh == 2 ) Bresenham(pInit, pFinal, area);

    }else
    {   
        gtk_label_set_label(GTK_LABEL(Widgets.label), "WARNING: There must be atleast 3 free points to draw a Polygon.");
        return False;
    }

    return True;
}

/**
 * @brief Creates and draws Lines with previously specified Drawing Algorithm.
 * 
 * @param area Drawing Area.
 * 
 * @returns True if code was correctly executed, False otherwise.
*/
Bool Line(GtkWidget *area)
{
    // If no algorithm selected, "Error".
    if ( algh == 0 )
    {
        gtk_label_set_label(GTK_LABEL(Widgets.label), "WARNING: You must provide the drawing algorithm.");
        return False;
    }

    int controller = number_taken_points(),
        size_p = array_get_curr_num(arr_points);


    if ( (size_p - controller) < 2 ) 
    {
        gtk_label_set_label(GTK_LABEL(Widgets.label), "WARNING: There must be atleast 2 free points to draw a Line.");
        return False;
    }

    while( controller < size_p )
    {
        /**
         * Must have atleast 2 points to draw a line,
         * If there is only one, "ignore it".
         */
        if ( (size_p - controller) >= 2 )
        {  
            struct point *pInit = array_get(arr_points, controller);
            struct point *pFinal = array_get(arr_points, controller + 1);

            if ( point_is_taken(pInit) ) continue;

            struct line *new_line = line_create(pInit, pFinal, algh);
            array_set(arr_lines, array_get_curr_num(arr_lines), new_line);
            point_take(pInit);
            point_take(pFinal);

            if ( algh == 1 ) DDA(pInit, pFinal, area);
            else if ( algh == 2 ) Bresenham(pInit, pFinal, area);
            controller++;
        } 
        controller++;
    }
    return True;
}

/**
 * @brief Retrieves the elapsed time by a function and prints it in "label".
 * 
 * @param t Elapsed time
*/
void write_execution_time(double t)
{
    char template[100],
         result[100];


    sprintf(template, "%f", (((double)t)/CLOCKS_PER_SEC)*100);
    strcpy(result, "Time Elapsed: ");
    strcat(result, template);
    strcat(result, "ms.\0");
    gtk_label_set_label(GTK_LABEL(Widgets.label), result);
}

void redraw_objects(GtkWidget *area)
{   
    int aux[MAX_POINTS],
        cont = 0;
    for (int i = 0; i < MAX_POINTS; i++ ) aux[i] = -2;

    clear_surface(0);
    gtk_widget_queue_draw(Widgets.drawing_area);
    cairo_t *cr;
    cr = cairo_create(surface);
    point_tt *points,
             *oposite;

    // Lines 
    for ( int i = 0; i < array_get_curr_num(arr_lines); i++ )    
    {
        struct line *line = array_get(arr_lines, i);
        int line_algh = line_get_algh(line);
        
        if ( line_was_clipped(line) == 0 ) 
        {
            points = line_get_points(line);

            // Preventing errors
            oposite = line_get_clipped_points(line);
            for ( int j = 0; j < 2; j++ )
                if ( oposite[j] != NULL ) aux[cont++] = point_id(oposite[j]);
        }
        else if ( line_was_clipped(line) == 1 )
        {
            points = line_get_clipped_points(line);

            // Preventing errors
            oposite = line_get_points(line);
            for ( int j = 0; j < 2; j++ ) {
                if ( oposite[j] != NULL ) aux[cont++] = point_id(oposite[j]);
            }
        }
        // Implementation decision. Whenever one of polygon's side is not IN the clip area, don't draw the polygon at all
         else if ( line_was_clipped(line) == 2 )
        {
            continue;
        }

        // Redrawing points
        for ( int j = 0; j < 2; j++ )
        {
            aux[cont++] = point_id(points[j]);
            double x = point_x_coord(points[j]),
                   y = point_y_coord(points[j]);
            draw_brush(Widgets.drawing_area, cr, x, y, color_get_colors(point_color(points[j])));
            draw_text(Widgets.drawing_area, cr, points[j]);
        }

        // Redrawing lines between points
        if ( line_algh == 1 ) DDA(points[0], points[1], Widgets.drawing_area);
        else if ( line_algh == 2 ) Bresenham(points[0], points[1], Widgets.drawing_area);

    }
     
    // Polygons
    for ( int i = 0; i < array_get_curr_num(arr_polygons); i++ )
    {
        struct polygon *pl = array_get(arr_polygons, i);
        int polygon_algh = polygon_get_algh(pl);
        array_tt p_points;
        array_tt p_oposite;

        if ( polygon_was_clipped(pl) == 0 ) 
        {
            p_points = polygon_get_points(pl);

            // Preventing errors
            p_oposite = polygon_get_clipped_points(pl);
            for ( int j = 0; j < array_get_curr_num(p_oposite); j++ )
                   if ( array_get(p_oposite, j) != NULL ) aux[cont++] = point_id(array_get(p_oposite, j));
        }
        else if ( polygon_was_clipped(pl) == 1 )
        {
            p_points = polygon_get_clipped_points(pl);

            // Preventing errors
            p_oposite = polygon_get_points(pl);
            for ( int j = 0; j < array_get_curr_num(p_oposite); j++ ) 
                {
                    aux[cont++] = point_id(array_get(p_oposite, j));
                }
        } 
        else if ( polygon_was_clipped(pl) == 2 )
        {
            continue;
        }

        // Redrawing points
        for ( int j = 0; j < array_get_curr_num(p_points) ; j++ )
        {   
            struct point *p = array_get(p_points, j);
            aux[cont++] = point_id(p);
            double x = point_x_coord(array_get(p_points, j)),
                   y = point_y_coord(array_get(p_points, j));
            draw_brush(Widgets.drawing_area, cr, x, y, color_get_colors(point_color(p)));
            draw_text(Widgets.drawing_area, cr, p);
        }

        // Redrawing lines between points
        for ( int j = 0; j < array_get_curr_num(p_points) - 1; j++ )
        {
            struct point *pInit = array_get(p_points, j);
            struct point *pFinal = array_get(p_points, j + 1);

            if ( algh == 1 ) DDA(pInit, pFinal, Widgets.drawing_area);
            else if ( algh == 2 ) Bresenham(pInit, pFinal, Widgets.drawing_area);
        }
        // Closing Polygon
        struct point *pInit = array_get(p_points, array_get_curr_num(p_points) - 1);
        struct point *pFinal = array_get(p_points, 0);

        if ( algh == 1 ) DDA(pInit, pFinal, Widgets.drawing_area);
        else if ( algh == 2 ) Bresenham(pInit, pFinal, Widgets.drawing_area);
    }

    // Circumference
    for ( int i = 0; i < array_get_curr_num(arr_circumferences); i++ )
    {
        circumference_tt circumference = array_get(arr_circumferences, i);

        points = circumference_get_points(circumference);

        for ( int j = 0; j < 2; j++ )
        {
            aux[cont++] = point_id(points[j]);
            double x = point_x_coord(points[j]),
                    y = point_y_coord(points[j]);
            point_set_coord(points[j], x, y);
            draw_brush(Widgets.drawing_area, cr, x, y, color_get_colors(point_color(points[j])));
            draw_text(Widgets.drawing_area, cr, points[j]);
        }

        calculate_circumference_points(Widgets.drawing_area, circumference);
    }

    // Clips
    for ( int i = 0; i < array_get_curr_num(arr_clips); i++ )
    {
        struct clip *cl = array_get(arr_clips, i);
        array_tt p_points;


        p_points = clip_get_points(cl);

        // Redrawing points
        for ( int j = 0; j < array_get_curr_num(p_points) ; j++ )
        {   
            struct point *p = array_get(p_points, j);
            aux[cont++] = point_id(p);
            double x = point_x_coord(array_get(p_points, j)),
                   y = point_y_coord(array_get(p_points, j));
            draw_brush(Widgets.drawing_area, cr, x, y, color_get_colors(point_color(p)));
            draw_text(Widgets.drawing_area, cr, p);
        }

        // Redrawing lines between points
        for ( int j = 0; j < array_get_curr_num(p_points) - 1; j++ )
        {
            struct point *pInit = array_get(p_points, j);
            struct point *pFinal = array_get(p_points, j + 1);

            DDA(pInit, pFinal, Widgets.drawing_area);
        }
        // Closing Polygon
        struct point *pInit = array_get(p_points, array_get_curr_num(p_points) - 1);
        struct point *pFinal = array_get(p_points, 0);

        DDA(pInit, pFinal, Widgets.drawing_area);
    }

    // Points
    // Drawing all points that aren't part of an object
    for ( int i = 0; i < array_get_curr_num(arr_points); i++ )
    {
        // drawn = False;
        point_tt p = array_get(arr_points, i);
        if ( p == NULL ) continue;
        int id = point_id(p);

        if ( !point_is_taken(p) ) 
        {
            draw_brush(Widgets.drawing_area, cr, point_x_coord(p), point_y_coord(p), color_get_colors(point_color(p)));
            draw_text(Widgets.drawing_area, cr, p);
        }
    }   
    cairo_destroy(cr);
}

/**
 * @brief (CALL_BACK) Function called whenever an option in "Drawing"drop-down is sellected, 
 * Defines which option of drawing (Line, Polygon or Circumfere) should be drawn.
 * 
 * @param dropdown Dropdown selected
 * @param entry 
*/
static void drawings_execution(GtkDropDown *dropdown,
                               GtkWidget   *area,
                               gpointer     user_data)
{
    int dropdown_selected = gtk_drop_down_get_selected(dropdown);
    clock_t t;
    Bool cntrl = False;

    GtkWidget *drawing_area = GTK_WIDGET(user_data);
    switch (dropdown_selected)
    {
        case 1:
            t = clock();
            cntrl = Line(drawing_area);
            t = clock() - t;
            if ( cntrl ) write_execution_time(t);
            break;
        case 2:
            t = clock();
            cntrl = Polygon(drawing_area);
            t = clock() - t;
            if ( cntrl ) write_execution_time(t);
            break;
        case 3:
            t = clock();
            cntrl = Circumference(drawing_area);
            t = clock() - t;
            if ( cntrl ) write_execution_time(t);
            break;
        default:
            break;
    }


}

char* read_from_until(char *content,
                      char  stop,
                      int  *pos)
{
    char *result = (char*) malloc(sizeof(char) * 100);
    int controller = 0;

    while ( content[*pos] != stop  )
    {
        // Ensuring that only numbers will be in our template
        if ( (content[*pos] < '0' || content[*pos] > '9') && content[*pos] != '.' && content[*pos] != '-' && content[*pos] != '+')
        {
            gtk_label_set_label(GTK_LABEL(Widgets.label), "WARNING: Template found is invalid. Please, reformulate your input.");
            return NULL;
        }
        result[controller++] = content[(*pos)++];
    }

    result[controller] = '\0';
    return result;
}

/**
 * @brief Checks if given content is correct based in given transformation algorithm's template (Check documentation).
 * If values[pos] == -1, it implies that this position is not used.
 *  
 * @param content   Content to be analysed
 * @param transf_id Which transformation was selected
 * 
 * @returns An array that contains the read values based on transformation algorithm's template (Check documentation). 
*/
double* check_content(char* content, 
                      int   transf_id)
{
    char *first  = (char*) malloc(sizeof(char) * 100),
         *second = (char*) malloc(sizeof(char) * 100),
         *foo = (char*) malloc(sizeof(char) * 100);
         
    int controller = 0;
    double *values = (double*) malloc(sizeof(double) * 3);
    // Translation and Scale. Should be informed as: '(X,Y)' - Where X and Y are, both, the integer values (pos or neg).
    if ( transf_id == 1 || transf_id == 3)
    {
        if ( strlen(content) < 5 )
        {
            if ( transf_id == 1 )
                gtk_label_set_label(GTK_LABEL(Widgets.label), "WARNING: Translation template is: '(INT,INT)'. Please, reformulate your input.");
            else gtk_label_set_label(GTK_LABEL(Widgets.label), "WARNING: Scale template is: '(INT,INT)'. Please, reformulate your input.");
            return NULL;
        }
        double x = 0.0f,
               y = 0.0f;
        controller = 1;
        first = read_from_until(content, ',', &controller);
        if ( first == NULL ) return NULL;

        x = strtod(first, &foo);
        controller++;

        second = read_from_until(content, ')', &controller);
        if ( second == NULL ) return NULL;

        y = strtod(second, &foo);
        values[0] = x;
        values[1] = y;
        values[2] = -1;
        return values;
    } 
    // Rotation. Should be informed as: 'Xd' - Where X is an integer rotation degree value (pos or neg).
    else if ( transf_id == 2 )
    {
        if ( strlen(content) > 5 )
        {
            gtk_label_set_label(GTK_LABEL(Widgets.label), "WARNING: Rotation template is: 'INTd'. Please, reformulate your input.");
            return NULL;
        }

        double ang = 0.0f;
        first = read_from_until(content, 'd', &controller);
        if ( first == NULL ) return NULL;

        ang = strtod(first, &foo);
        values[0] = ang * M_PI / 180.0;
        values[1] = -1;
        values[2] = -1;
        return values;
    }


    // House Keeping
    free(first);
    free(second);
    free(foo);
}

/**
 * @brief Applies the XYReflection in all points of all drawn objects (except Clips). After changing values, calls "redraw_objects" function to redraw everything at new position.
 * 
 * @return True if code execution was correct. False otherwise
*/
Bool xyreflection()
{
    // Lines.
    for ( int i = 0; i < array_get_curr_num(arr_lines); i++ )
    {
        line_tt foo = array_get(arr_lines, i);

        point_tt *points = line_get_points(foo);

        for ( int j = 0; j < 2; j++ )
        {
            double new_x = -point_x_coord(points[j]),
                   new_y = -point_y_coord(points[j]);
            point_set_coord(points[j], new_x, new_y);
        }
    }

    // Polygons.
    for ( int i = 0; i < array_get_curr_num(arr_polygons); i++ )
    {
        polygon_tt foo = array_get(arr_polygons, i);

        array_tt p_points = polygon_get_points(foo);

        for ( int j = 0; j < array_get_curr_num(p_points) ; j++ )
        {   
            struct point *p = array_get(p_points, j);
            double new_x = -point_x_coord(array_get(p_points, j)),
                   new_y = -point_y_coord(array_get(p_points, j));
            point_set_coord(p, new_x, new_y);
        }
    }
    
    // Circumference.
    for ( int i = 0; i < array_get_curr_num(arr_circumferences); i++ )
    {
        circumference_tt foo = array_get(arr_circumferences, i);

        point_tt *points = circumference_get_points(foo);

        for ( int j = 0; j < 2; j++ )
        {
            double new_x = -point_x_coord(points[j]),
                   new_y = -point_y_coord(points[j]);
            point_set_coord(points[j], new_x, new_y);
        }
    }

    redraw_objects(Widgets.drawing_area);
    return True;
}

/**
 * @brief Applies the YReflection in all points of all drawn objects (except Clips). After changing values, calls "redraw_objects" function to redraw everything at new position.
 * 
 * @return True if code execution was correct. False otherwise
*/
Bool yreflection()
{
    // Lines.
    for ( int i = 0; i < array_get_curr_num(arr_lines); i++ )
    {
        line_tt foo = array_get(arr_lines, i);

        point_tt *points = line_get_points(foo);

        for ( int j = 0; j < 2; j++ )
        {
            double new_x = -point_x_coord(points[j]),
                   new_y = point_y_coord(points[j]);
            point_set_coord(points[j], new_x, new_y);
        }
    }

    // Polygons.
    for ( int i = 0; i < array_get_curr_num(arr_polygons); i++ )
    {
        polygon_tt foo = array_get(arr_polygons, i);

        array_tt p_points = polygon_get_points(foo);

        for ( int j = 0; j < array_get_curr_num(p_points) ; j++ )
        {   
            struct point *p = array_get(p_points, j);
            double new_x = -point_x_coord(array_get(p_points, j)),
                   new_y = point_y_coord(array_get(p_points, j));
            point_set_coord(p, new_x, new_y);;
        }
    }

    // Circumference.
    for ( int i = 0; i < array_get_curr_num(arr_circumferences); i++ )
    {
        circumference_tt foo = array_get(arr_circumferences, i);

        point_tt *points = circumference_get_points(foo);

        for ( int j = 0; j < 2; j++ )
        {
            double new_x = -point_x_coord(points[j]),
                   new_y = point_y_coord(points[j]);
            point_set_coord(points[j], new_x, new_y);
        }
    }
    redraw_objects(Widgets.drawing_area);
    return True;
}

/**
 * @brief Applies the XReflection in all points of all drawn objects (except Clips). After changing values, calls "redraw_objects" function to redraw everything at new position.
 * 
 * @return True if code execution was correct. False otherwise
*/
Bool xreflection()
{    
    // Lines.
    for ( int i = 0; i < array_get_curr_num(arr_lines); i++ )
    {
        
        line_tt foo = array_get(arr_lines, i);

        point_tt *points = line_get_points(foo);

        for ( int j = 0; j < 2; j++ )
        {
            double new_x = point_x_coord(points[j]),
                   new_y = -point_y_coord(points[j]);
            point_set_coord(points[j], new_x, new_y);
        }
    }

    // Polygons
    for ( int i = 0; i < array_get_curr_num(arr_polygons); i++ )
    {
        polygon_tt foo = array_get(arr_polygons, i);

        array_tt p_points = polygon_get_points(foo);

        for ( int j = 0; j < array_get_curr_num(p_points) ; j++ )
        {   
            struct point *p = array_get(p_points, j);
            double new_x = point_x_coord(array_get(p_points, j)),
                   new_y = -point_y_coord(array_get(p_points, j));
            point_set_coord(p, new_x, new_y);
        }
    }

    // Circumference.
    for ( int i = 0; i < array_get_curr_num(arr_circumferences); i++ )
    {
        circumference_tt foo = array_get(arr_circumferences, i);

        point_tt *points = circumference_get_points(foo);

        for ( int j = 0; j < 2; j++ )
        {
            double new_x = point_x_coord(points[j]),
                   new_y = -point_y_coord(points[j]);
            point_set_coord(points[j], new_x, new_y);
        }
    }

    redraw_objects(Widgets.drawing_area);
    return True;
}


/**
 * @brief Pins the first point and applies the Rotation in each point (besides the pinned one) of all drawn objects (except Clips). After changing values, calls "redraw_objects" function to redraw everything at new position.
 * Since points are pinned, circumference won't be rotated (Since the first point is the center of the circumference).
 * 
 * @return True if code execution was correct. False otherwise
*/
Bool rotation(char *content,
              int   transf_id)
{
    // Sanity Check
    if ( strlen(content) == 0 ) 
    {
        gtk_label_set_label(GTK_LABEL(Widgets.label), "WARNING: You must provide paramethers to this transformation.");
        return False;
    }
    
    double *rotation = NULL;
    if ( (rotation = check_content(content, transf_id)) == NULL ) return False;

    // Pinning first point always
    // Lines.
    for ( int i = 0; i < array_get_curr_num(arr_lines); i++ )
    {
        line_tt foo = array_get(arr_lines, i);

        point_tt *points = line_get_points(foo);

        double pinned_x = point_x_coord(points[0]),
               pinned_y = point_y_coord(points[0]);

        for ( int j = 1; j < 2; j++ )
        {
            double new_x = ( (point_x_coord(points[j]) - pinned_x) * cos(rotation[0])) - ( (point_y_coord(points[j]) - pinned_y) * sin(rotation[0])),
                   new_y = ( (point_x_coord(points[j]) - pinned_x) * sin(rotation[0])) + ( (point_y_coord(points[j]) - pinned_y) * cos(rotation[0]));
            point_set_coord(points[j], new_x + pinned_x, new_y + pinned_y);
        }
        
    }

    // Polygons
    for ( int i = 0; i < array_get_curr_num(arr_polygons); i++ )
    {
        polygon_tt foo = array_get(arr_polygons, i);

        array_tt p_points = polygon_get_points(foo);

        point_tt p_pinned = array_get(p_points, 0);
        double pinned_x = point_x_coord(p_pinned),
               pinned_y = point_y_coord(p_pinned);

        for ( int j = 1; j < array_get_curr_num(p_points) ; j++ )
        {   
            struct point *p = array_get(p_points, j);
            double new_x = ( (point_x_coord(p) - pinned_x) * cos(rotation[0])) - ( (point_y_coord(p) - pinned_y) * sin(rotation[0])),
                   new_y = ( (point_x_coord(p) - pinned_x) * sin(rotation[0])) + ( (point_y_coord(p) - pinned_y) * cos(rotation[0]));
            point_set_coord(p, new_x + pinned_x, new_y + pinned_y);
        }
    }

    redraw_objects(Widgets.drawing_area);
    return True;
}

/**
 * @brief Applies the Scale in all points of all drawn objects (except Clips). After changing values, calls "redraw_objects" function to redraw everything at new position.
 * When values specified are negative, it means to SHRINK "the objetct". Positive values means to increase it.
 * 
 * @return True if code execution was correct. False otherwise
*/
Bool scale(char *content, 
           int   transf_id)
{
    // Sanity Check
    if ( strlen(content) == 0 ) 
    {
        gtk_label_set_label(GTK_LABEL(Widgets.label), "WARNING: You must provide paramethers to this transformation.");
        return False;
    }

    // When Scaling, whenever a value is negative, it means to SHRINK "the object". Positive values means to increase
    double *scale = NULL;
    if ( (scale = check_content(content, transf_id)) == NULL ) return False;


    // Lines
    for ( int i = 0; i < array_get_curr_num(arr_lines); i++ )
    {
        line_tt foo = array_get(arr_lines, i);
        point_tt *points = line_get_points(foo);

        for ( int j = 0; j < 2; j++ )
        {
            double new_x = point_x_coord(points[j]),
                   new_y = point_y_coord(points[j]);
            int line_algh = line_get_algh(foo);
            if ( scale[0] < 0 ) new_x /= abs(scale[0]);
            else new_x *= scale[0];
            if ( scale[1] < 0 ) new_y /= abs(scale[1]);
            else new_y *= scale[1]; 

            point_set_coord(points[j], new_x, new_y);
        }
    }

    // Polygons
    for ( int i = 0; i < array_get_curr_num(arr_polygons); i++ )
    {
        polygon_tt foo = array_get(arr_polygons, i);

        array_tt p_points = polygon_get_points(foo);

        for ( int j = 0; j < array_get_curr_num(p_points) ; j++ )
        {   
            struct point *p = array_get(p_points, j);
            double new_x = point_x_coord(array_get(p_points, j)),
                   new_y = point_y_coord(array_get(p_points, j));

            if ( scale[0] < 0 ) new_x /= abs(scale[0]);
            else new_x *= scale[0];
            if ( scale[1] < 0 ) new_y /= abs(scale[1]);
            else new_y *= scale[1]; 
            point_set_coord(p, new_x, new_y);
        }
    }

    // Circumference.
    for ( int i = 0; i < array_get_curr_num(arr_circumferences); i++ )
    {
        circumference_tt foo = array_get(arr_circumferences, i);

        point_tt *points = circumference_get_points(foo);

        for ( int j = 0; j < 2; j++ )
        {
            double new_x = point_x_coord(points[j]),
                   new_y = point_y_coord(points[j]);

            if ( scale[0] < 0 ) new_x /= abs(scale[0]);
            else new_x *= scale[0];
            if ( scale[1] < 0 ) new_y /= abs(scale[1]);
            else new_y *= scale[1]; 

            point_set_coord(points[j], new_x, new_y);
        }
    }
    redraw_objects(Widgets.drawing_area);

    free(scale);
    return True;


}

/**
 * @brief Applies the Translation in all points of all drawn objects (except Clips). After changing values, calls "redraw_objects" function to redraw everything at new position.
 * 
 * @return True if code execution was correct. False otherwise
*/
Bool translation(char *content,
                 int   transf_id)
{
    // Sanity Check
    if ( strlen(content) == 0 ) 
    {
        gtk_label_set_label(GTK_LABEL(Widgets.label), "WARNING: You must provide paramethers to this transformation.");
        return False;
    }

    double *translation = NULL;
    if ( (translation = check_content(content, transf_id)) == NULL ) return False;

    // Lines 
    for ( int i = 0; i < array_get_curr_num(arr_lines); i++ )
    {
        line_tt foo = array_get(arr_lines, i);

        point_tt *points = line_get_points(foo);

        for ( int j = 0; j < 2; j++ )
        {
            double new_x = point_x_coord(points[j]) + translation[0],
                   new_y = point_y_coord(points[j]) + translation[1];
            point_set_coord(points[j], new_x, new_y);
        }        
    }

    // Polygons
    for ( int i = 0; i < array_get_curr_num(arr_polygons); i++ )
    {
        polygon_tt foo = array_get(arr_polygons, i);

        array_tt p_points = polygon_get_points(foo);

        for ( int j = 0; j < array_get_curr_num(p_points) ; j++ )
        {   
            struct point *p = array_get(p_points, j);
            double new_x = point_x_coord(p) + translation[0],
                   new_y = point_y_coord(p) + translation[1];
            point_set_coord(p, new_x, new_y);
        }
    }

    // Circumference.
    for ( int i = 0; i < array_get_curr_num(arr_circumferences); i++ )
    {
        circumference_tt foo = array_get(arr_circumferences, i);

        point_tt *points = circumference_get_points(foo);

        for ( int j = 0; j < 2; j++ )
        {
            double new_x = point_x_coord(points[j]) + translation[0],
                   new_y = point_y_coord(points[j]) + translation[1];
            point_set_coord(points[j], new_x, new_y);
        }
    }

    redraw_objects(Widgets.drawing_area);
 
    free(translation);
    return True;
}

/**
 * @brief (CALL_BACK) Function called whenever an option in "Transformation"drop-down is sellected, also, the value filled in "entry" is used.
 * Defines which option of transformation (Translation, rotation, scale or refleciton) should be done.
 * 
 * @param dropdown Dropdown selected
 * @param entry 
*/
static void transformation_execution(GtkDropDown *dropdown,
                                     GtkEntry    *entry,
                                     gpointer     user_data)
{
    // Getting the content written into the entry. It's allocated in a reserved buffer
    char *content = g_strdup(gtk_entry_buffer_get_text(user_data));
    int dropdown_selected = gtk_drop_down_get_selected(dropdown);
    clock_t t;
    Bool cntrl;

    if ( dropdown_selected != 0 && array_get_curr_num(arr_points) == 0 )
    {
        gtk_label_set_label(GTK_LABEL(Widgets.label), "WARNING: You must draw points to perform transformations.");
        return;
    } else if ( dropdown_selected != 0 && array_get_curr_num(arr_lines) == 0 && array_get_curr_num(arr_polygons) == 0 && array_get_curr_num(arr_circumferences) == 0 )
    {
        gtk_label_set_label(GTK_LABEL(Widgets.label), "WARNING: You must draw an object to perform transformations.");
        return;
    }

    switch (dropdown_selected)
    {
        case 1:
            t = clock();
            cntrl = translation(content, dropdown_selected);
            t = clock() - t;
            if ( cntrl ) write_execution_time(t);
            break;
        case 2:
            t = clock();
            cntrl = rotation(content, dropdown_selected);
            t = clock() - t;
            if ( cntrl ) write_execution_time(t);
            break;
        case 3:
            t = clock();
            cntrl = scale(content, dropdown_selected);
            t = clock() - t;
            if ( cntrl ) write_execution_time(t);
            break;
        case 4:
            t = clock();
            cntrl = xreflection();
            t = clock() - t;
            if ( cntrl ) write_execution_time(t);
            break;
        case 5: 
            t = clock();
            cntrl = yreflection();
            t = clock() - t;
            if ( cntrl ) write_execution_time(t);
            break;
        case 6:
            t = clock();
            cntrl = xyreflection();
            t = clock() - t;
            if ( cntrl ) write_execution_time(t);
            break;
        default:
            gtk_label_set_label(GTK_LABEL(Widgets.label), "Debug informations here...");
            break;
    }

}


/**
 * @brief Creates a CLIP structure based on two points drawn by the user. Since the CLIP must have a rectangle shape, the other two points are created based on X and Y of drawn points.
 * Drawing algorithm used is DDA.
 * 
 * @return True if code execution was correct. False otherwise
*/
Bool clip_structure()
{
    int controller = number_taken_points(),
        size_p = array_get_curr_num(arr_points),
        iterator = 0;

    if ( array_get_curr_num(arr_clips) >= 1 )
    {
        gtk_label_set_label(GTK_LABEL(Widgets.label), "WARNING: You can have only 1 clip area per time.");
        return True;
    }

    if ( controller + 4 >= MAX_POINTS )
    {
        gtk_label_set_label(GTK_LABEL(Widgets.label), "WARNING: The number of points will surpass the MAX.");
        return False;
    }

    struct point **points = (point_tt*) malloc(sizeof(point_tt) * 4);
    struct point **aux = (point_tt*) malloc(sizeof(point_tt) * MAX_POINTS);
    if ( (size_p - controller) >= 2 )
    { 
        // Adding all left points into Clip's structure.
        while ( iterator < 2 ) 
        {
            point_tt p = array_get(arr_points, controller++);
            point_take(p);
            point_define_color(p, 255.0, 0.0, 0.0);
            points[iterator++] = p;
        }
        point_tt p_first = points[0],
                 p_third = points[1];


        // It must be a perfect rectangle, so User must point the two oposit points, and the others will be generated based on clicked point's coordinates.
        point_tt p_second = point_create(point_x_coord(p_third), point_y_coord(p_first));
        point_tt p_fourth = point_create(point_x_coord(p_first), point_y_coord(p_third));

        // Clip's points will always be drawn as red points, just to differentiate from others.
        point_define_color(p_second, 255.0, 0.0, 0.0);
        point_define_color(p_fourth, 255.0, 0.0, 0.0);
        point_take(p_second);
        point_take(p_fourth);
        iterator += 2;

        point_tt temp = p_third;
        points[0] = p_first;
        points[1] = p_second;
        points[2] = temp;
        points[3] = p_fourth;

        int i = 0,
            idx = 0,
            j = 0;
            controller = 0;   

        struct clip *clip = clip_create(points, iterator, algh);

        array_set(arr_clips, 0, clip);

        redraw_objects(Widgets.drawing_area);

        for ( int i = 0; i < iterator - 1; i++ )
        {
            struct point *pInit = points[i];
            struct point *pFinal = points[i + 1];
            point_take(pInit);

            // Using only DDA, idk why.
            DDA(pInit, pFinal, Widgets.drawing_area);
        }
        // Closing Clip
        struct point *pInit = points[iterator - 1];
        struct point *pFinal = points[0];

        DDA(pInit, pFinal, Widgets.drawing_area);

    }else
    {   
        gtk_label_set_label(GTK_LABEL(Widgets.label), "WARNING: There must be atleast 2 free points to draw a Clip.");
        return False;
    }

    return True;
}

/**
 * @brief Checks where a given DELTA(x) and DELTA(y) are related to CLIP area.
 * 
 * @param p  DELTA(X)
 * @param q  DELTA(Y)
 * 
 * @return True if code execution was correct. False otherwise
*/
Bool clip_test(double  p,
               double  q,
               double *u1,
               double *u2)
{
    Bool result = True;
    double r = 0.0f;

    // Inwards - outwards
    if ( p < 0.0f )
    {
        r = q / p;
        if ( r > *u2 ) result = False;
        else if ( r > *u1 ) *u1 = r;
    } 
    // Outwards - Inwards
    else if ( p > 0.0f )
    {
        r = q / p;
        if ( r < *u1 ) result = False;
        else if ( r < *u2 ) *u2 = r;
    } 
    else if ( q < 0 ) result = False;

    return result;
}

/**
 * @brief Uses Liang Barsky's algorithm to recalculate Line's points inside a Clipped area. 
 * 
 * @param pInit  Initial Line's point.
 * @param pFinal Final Line's point.
 * @param xmin   Clip's XMIN.
 * @param xmax   Clip's XMAX.
 * @param ymin   Clip's YMIN.
 * @param ymax   Clip's YMAX.
 * @param flag   Flag. > 1 = Create just the first point (clipped_pInit), 0 = Create both points (clipped_pInit clipped_pFinal)
 * 
 * @return New clipped points if line inside CLIPPED area. Null otherwise.
*/
point_tt* liang_barsky(struct point *pInit,
                       struct point *pFinal,
                       int           xmin,
                       int           xmax,
                       int           ymin,
                       int           ymax, 
                       int           flag)
{
    // Because of Float-precision, values had to be rounded, otherwise code would not work.
    int x1 = (int) point_x_coord(pInit),
        x2 = (int) point_x_coord(pFinal),
        y1 = (int) point_y_coord(pInit), 
        y2 = (int) point_y_coord(pFinal);

    double u1 = 0.0f,
           u2 = 1.0f;
    double dx = (double) x2 - x1,
           dy = (double) y2 - y1;

    point_tt *points = (point_tt*) malloc(sizeof(point_tt) * 2);
    point_tt c_pInit,
             c_pFinal;
    if ( clip_test(-dx, x1 - xmin, &u1, &u2) )
    {
        if ( clip_test(dx, xmax - x1, &u1, &u2) )
        {
            if ( clip_test(-dy, y1 - ymin, &u1, &u2) )
            {
                if ( clip_test(dy, ymax - y1, &u1, &u2 ) )
                {
                    if ( floor(u2) < 1.0f )
                    {
                        x2 = (int) x1 + u2 * dx;
                        y2 = (int) y1 + u2 * dy;
                    }
                    if ( ceil(u1) > 0.0f )
                    {
                        x1 = (int) x1 + u1 * dx;
                        y1 = (int) y1 + u1 * dy;
                    }
                    if ( flag >= 0 )
                    {
                        c_pInit = point_create((double) x1, (double)y1);
                        points[0] = c_pInit;
                        if ( flag == 0 )
                        {
                            c_pFinal = point_create((double)x2, (double)y2);
                            points[1] = c_pFinal;
                            point_define_color(c_pFinal, 0.0, 0.0, 0.0);
                            point_take(c_pFinal);
                            array_set(arr_points, array_get_curr_num(arr_points), c_pFinal);
                        }
                        point_define_color(c_pInit, 0.0, 0.0, 0.0);
                        point_take(c_pInit);

                        array_set(arr_points, array_get_curr_num(arr_points), c_pInit); 
                    }
                    return points;
                }
            }
        }
    }
    return NULL;
}

/**
 * @brief Initializes Clip Structure and operates Liang Barsky's algorithm in (already created) Lines and Polygons.
 * 
 * @return True if code execution was correct. False otherwise
*/
Bool liang_barsky_init()
{
    if ( !clip_structure() ) return False;
    if ( array_get_curr_num(arr_clips) == 0 ) return False;

    struct clip *clip = array_get(arr_clips, 0);
    double *maxmin = clip_get_maxmin(clip);

    int xmin = (int) maxmin[0],
        xmax = (int) maxmin[1],
        ymin = (int) maxmin[2],
        ymax = (int) maxmin[3];
    for ( int i = 0; i < array_get_curr_num(arr_lines); i++ )
    {
        
        struct line *foo = array_get(arr_lines, i);
        point_tt *points = line_get_points(foo);

        struct point *pInit = points[0];
        struct point *pFinal = points[1];
        // Whenever you call liang barsky, a new point will be CREATE!!
        // If you don't want that, make sure to clean the canvas first.
        points = liang_barsky(pInit, pFinal, xmin, xmax, ymin, ymax, 0);
        if ( points != NULL ) line_add_clipped_points(foo, points[0], points[1], 1);
        else line_add_clipped_points(foo, NULL, NULL, 2);

    } 

    struct point *pInit;
    struct point *pFinal;
    Bool aux = False;
    for ( int i = 0; i < array_get_curr_num(arr_polygons); i++ )
    {
        aux = False;
        struct polygon *foo = array_get(arr_polygons, i);
        point_tt *points = (point_tt*) malloc(sizeof(point_tt) * 2);
        array_tt c_points = polygon_get_points(foo);
        
        // Whenever you call liang barsky, a new point will be CREATE!!
        // If you don't want that, make sure to clean the canvas first.
        for ( int j = 0; j < array_get_curr_num(c_points) - 1; j++ )
        {
            pInit = array_get(c_points, j);
            pFinal = array_get(c_points, j + 1);
            points = liang_barsky(pInit, pFinal, xmin, xmax, ymin, ymax, 1);
            if ( points != NULL ) polygon_add_clipped_points(foo, points, 1, 1);
            else
            { 
                aux = True;
                polygon_add_clipped_points(foo, NULL, 1, 2);
                break;
            }
        }
        if ( aux ) continue;
        pInit = array_get(c_points, array_get_curr_num(c_points) - 1);
        pFinal = array_get(c_points, 0);
        points = liang_barsky(pInit, pFinal, xmin, xmax, ymin, ymax, 1);
        if ( points != NULL ) polygon_add_clipped_points(foo, points, 1, 1);
        else
        {
            polygon_add_clipped_points(foo, NULL, 1, 2);        
            continue;
        }
    }    redraw_objects(Widgets.drawing_area);
    free(maxmin);
    return True;
}

/**
 * @brief Calculate the "Binary" code based in Point XY and where they are related to CLIP area, used in Cohen Sutherland's algorithm.
 * 
 * @param x    Point's X coordinate.
 * @param y    Point's Y coordinate.
 * @param xmin Clips's XMIN.
 * @param xmax Clips's XMAX.
 * @param ymin Clips's YMIN.
 * @param ymax Clips's YMAX.
 * 
 * @returns 
*/
int region_code(int x, 
                int y,
                int xmin,
                int xmax,
                int ymin, 
                int ymax)
{
    int codigo = 0;

    // Left - Bit 0
    if ( x < xmin ) codigo += 1;
    // Right - Bit 1
    if ( x > xmax ) codigo += 2;
    // Down - Bit 2
    if ( y < ymin ) codigo += 4;
    // Up - Bit 3
    if ( y > ymax ) codigo += 8;

    return codigo;
}

/**
 * @brief Uses Cohen Sutherland's algorithm to recalculate Line's points inside a Clipped area. 
 * 
 * @param pInit  Initial Line's point.
 * @param pFinal Final Line's point.
 * @param xmin   Clip's XMIN.
 * @param xmax   Clip's XMAX.
 * @param ymin   Clip's YMIN.
 * @param ymax   Clip's YMAX.
 * @param flag   Flag. > 1 = Create just the first point (clipped_pInit), 0 = Create both points (clipped_pInit clipped_pFinal)
 * 
 * @return New clipped points if line inside CLIPPED area. Null otherwise.
*/
point_tt* cohen_sutherland(struct point *pInit,
                           struct point *pFinal,
                           int           xmin,
                           int           xmax,
                           int           ymin,
                           int           ymax, 
                           int           flag)
{
    // Because of Float-precision, values had to be rounded, otherwise code would not work.
    int x1 = (int) point_x_coord(pInit),
        x2 = (int) point_x_coord(pFinal),
        y1 = (int) point_y_coord(pInit), 
        y2 = (int) point_y_coord(pFinal);

    int c1 = 0,
        c2 = 0,
        cfora = 0;
    int xint = 0,
        yint = 0;
    Bool aceite = False,
         feito = False;
    
    point_tt *points = (point_tt*) malloc(sizeof(point_tt) * 2);
    struct point *c_pInit;
    struct point *c_pFinal;

    while ( !feito )
    {   
        c1 = region_code(x1, y1, xmin, xmax, ymin, ymax);
        c2 = region_code(x2, y2, xmin, xmax, ymin, ymax);
        // Fully-in
        if ( c1 == 0 && c2 == 0 )
        {    
            if ( flag >= 0 )
            {
                c_pInit = point_create((double) x1, (double)y1);
                points[0] = c_pInit;
                if ( flag == 0 )
                {
                    c_pFinal = point_create((double)x2, (double)y2);
                    points[1] = c_pFinal;
                    point_define_color(c_pFinal, 0.0, 0.0, 0.0);
                    point_take(c_pFinal);
                    array_set(arr_points, array_get_curr_num(arr_points), c_pFinal);
                }
                point_define_color(c_pInit, 0.0, 0.0, 0.0);
                point_take(c_pInit);

                array_set(arr_points, array_get_curr_num(arr_points), c_pInit); 
            }
            


            aceite = True;
            feito = True;
        } 
        // Segment Fully-out
        else if ( (c1 & c2) != 0 )
        {
            feito = True;
        } else 
        {
            if ( c1 != 0 ) {
                cfora = c1;

            } 
            else
            {
                cfora = c2;
            }


            // Left Lim
            if ( (cfora & 1) == 1 )
            {
                xint = xmin;
                yint = (int) y1 + ( y2 - y1 ) * ( xmin - x1 ) / ( x2 - x1 );
            }
            // Right Lim
            else if ( (cfora & 2) == 2 )
            {
                // g_print("Entrei aqui né krl\n");
                xint = xmax; 
                yint = (int) y1 + ( y2 - y1 ) * ( xmax - x1 ) / ( x2 - x1 );
            }
            // Dowm Lim
            else if ( (cfora & 4) == 4)
            {
                yint = ymin;
                xint = (int) x1 + ( x2 - x1 ) * ( ymin - y1 ) / ( y2 - y1 );
            }
            // Up Lim.
            else if ( (cfora & 8) == 8)
            {
                yint = ymax;
                xint = (int) x1 + ( x2 - x1 ) * ( ymax - y1 ) / ( y2 - y1 );
            }

            if ( cfora == c1 )
            {
                x1 = round(xint);
                y1 = round(yint);
            } else
            {
                x2 =round(xint);
                y2 = round(yint);
            }
        }
    }

    if ( aceite )
    {   
        return points;    
    }
    return NULL;
} 

/**
 * @brief Initializes Clip Structure and operates Cohen Sutherland's algorithm in (already created) Lines and Polygons.
 * 
 * @return True if code execution was correct. False otherwise
*/
Bool cohen_init()
{
    if ( !clip_structure() ) return False;
    if ( array_get_curr_num(arr_clips) == 0 ) return False;

    struct clip *clip = array_get(arr_clips, 0);
    double *maxmin = clip_get_maxmin(clip);

    int xmin = (int) maxmin[0],
        xmax = (int) maxmin[1],
        ymin = (int) maxmin[2],
        ymax = (int) maxmin[3];
    for ( int i = 0; i < array_get_curr_num(arr_lines); i++ )
    {
        
        struct line *foo = array_get(arr_lines, i);
        point_tt *points = line_get_points(foo);

        struct point *pInit = points[0];
        struct point *pFinal = points[1];
        // Whenever you call cohen_sutherlnad, a new point will be CREATE!!
        // If you don't want that, make sure to clean the canvas first.
        points = cohen_sutherland(pInit, pFinal, xmin, xmax, ymin, ymax, 0);
        if ( points != NULL ) line_add_clipped_points(foo, points[0], points[1], 1);
        else line_add_clipped_points(foo, NULL, NULL, 2);

    } 

    struct point *pInit;
    struct point *pFinal;
    Bool aux = False;
    for ( int i = 0; i < array_get_curr_num(arr_polygons); i++ )
    {
        aux = False;
        struct polygon *foo = array_get(arr_polygons, i);
        point_tt *points = (point_tt*) malloc(sizeof(point_tt) * 2);
        array_tt c_points = polygon_get_points(foo);

        for ( int j = 0; j < array_get_curr_num(c_points) - 1; j++ )
        {
            pInit = array_get(c_points, j);
            pFinal = array_get(c_points, j + 1);
            // Whenever you call cohen_sutherlnad, a new point will be CREATE!!
            // If you don't want that, make sure to clean the canvas first.
            points = cohen_sutherland(pInit, pFinal, xmin, xmax, ymin, ymax, 1);
            if ( points != NULL ) polygon_add_clipped_points(foo, points, 1, 1);
            else
            { 
                aux = True;
                polygon_add_clipped_points(foo, NULL, 1, 2);
                break;
            }
        }
        if ( aux ) continue;
        pInit = array_get(c_points, array_get_curr_num(c_points) - 1);
        pFinal = array_get(c_points, 0);
        points = cohen_sutherland(pInit, pFinal, xmin, xmax, ymin, ymax, 1);
        if ( points != NULL ) polygon_add_clipped_points(foo, points, 1, 1);
        else
        {
            polygon_add_clipped_points(foo, NULL, 1, 2);        
            continue;
        }
    }


    redraw_objects(Widgets.drawing_area);
    free(maxmin);
    return True;
}

/**
 * @brief (CALL_BACK) Function called whenever an option in "Croppings"drop-down is sellected.
 * Defines which image cropping algorithm must be used. Also, when an algorithm is selected, points color should change to represent the cropping area.
 * 
 * @param dropdown Dropdown selected
*/
static void cropping_selection(GtkDropDown *dropdown,
                               gpointer     user_data)
{
    int dropdown_selected = gtk_drop_down_get_selected(dropdown);

    if ( dropdown_selected != 0 && array_get_curr_num(arr_points) == 0 )
    {
        gtk_label_set_label(GTK_LABEL(Widgets.label), "WARNING: You must draw points to perform clipping.");
        return;
    }
    clock_t t;
    Bool cntrl;

    switch (dropdown_selected)
    {
        case 1: 
            t = clock();
            cntrl = cohen_init();
            t = clock() - t;
            if ( cntrl ) write_execution_time(t);
            break;
        case 2:
            t = clock();
            cntrl = liang_barsky_init();
            t = clock() - t;
            if ( cntrl ) write_execution_time(t);
            break;
        default:

    }
}   

/**
 * @brief Used to get User's screen config in order to draw canvas in a bigger size.
 * 
 * @param width  Screen's width.
 * @param height Screen's height.
*/
void user_monitor_info(int *width, int *height)
{
    Display *display = XOpenDisplay(NULL);

    assert( display );
    Window root = DefaultRootWindow(display);
    XRRScreenResources *res = XRRGetScreenResources(display, root);

    // Forcing the choosen monitor to be the first one that appears
    XRROutputInfo *output_info = XRRGetOutputInfo(display, res, res->outputs[0]);
    assert( output_info->connection == RR_Connected );
    XRRCrtcInfo *crtc_info = XRRGetCrtcInfo(display, res, output_info->crtc);

    assert( crtc_info );
    *width = crtc_info->width;
    *height = crtc_info->height;

    XRRFreeCrtcInfo(crtc_info);
    XRRFreeOutputInfo(output_info);
    XRRFreeScreenResources(res);
    XCloseDisplay(display);
}


/**
 * @brief Main function, controlls all Widgets and Gestures
 * 
 * @param app Main application
*/
static void activate(GtkApplication *app, 
                     gpointer        user_data)
{
    const char *dropdown_content_algorithms[4] = {"Drawing Algorithms\0", "DDA\0", "Bresenham\0"};
    const char *dropdown_content_drawings[5] = {"Objects\0", "Line\0", "Polygon\0", "Circumference\0"};
    const char *dropdown_content_transformations[8] = {"Geometric Transformations\0", "Translate\0", "Rotate\0", "Scale\0", "X Reflection\0", "Y Reflection\0", "XY Reflection\0"};
    const char *dropdown_content_croppings[5] = {"Clipping Algorithms\0", "Cohen-Sutherland\0", "Liang-Barsky\0"};

    int width,
        height;

    user_monitor_info(&width, &height);

    Widgets.window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(Widgets.window), "GC Project");

    g_signal_connect(Widgets.window, "destroy", G_CALLBACK(close_window), NULL);

    
    Widgets.main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_window_set_child(GTK_WINDOW(Widgets.window), Widgets.main_box);

    Widgets.frame_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_append(GTK_BOX(Widgets.main_box), Widgets.frame_box);

    Widgets.frame = gtk_frame_new(NULL);
    gtk_box_append(GTK_BOX(Widgets.frame_box), Widgets.frame);

    Widgets.drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(Widgets.drawing_area, width, ((height * 91) / 100));
    gtk_frame_set_child(GTK_FRAME(Widgets.frame), Widgets.drawing_area);

    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(Widgets.drawing_area), draw_cb, NULL, NULL);
    g_signal_connect_after(Widgets.drawing_area, "resize", G_CALLBACK(resize_cb), NULL);

    Widgets.drag = gtk_gesture_drag_new();
    gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(Widgets.drag), GDK_BUTTON_PRIMARY);
    gtk_widget_add_controller(Widgets.drawing_area, GTK_EVENT_CONTROLLER(Widgets.drag));
    g_signal_connect(Widgets.drag, "drag-begin", G_CALLBACK(draw), Widgets.drawing_area); 

    Widgets.press = gtk_gesture_click_new();
    gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(Widgets.press), GDK_BUTTON_SECONDARY);
    gtk_widget_add_controller(Widgets.drawing_area, GTK_EVENT_CONTROLLER(Widgets.press));

    g_signal_connect(Widgets.press, "pressed", G_CALLBACK(clean), Widgets.drawing_area);
    
    Widgets.option_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_append(GTK_BOX(Widgets.main_box), Widgets.option_box);
    gtk_box_set_spacing(GTK_BOX(Widgets.option_box), 5);

    Widgets.dropdown_algorithms = gtk_drop_down_new_from_strings(dropdown_content_algorithms);
    gtk_box_append(GTK_BOX(Widgets.option_box), Widgets.dropdown_algorithms);

    Widgets.dropdown_drawings = gtk_drop_down_new_from_strings(dropdown_content_drawings);
    gtk_box_append(GTK_BOX(Widgets.option_box), Widgets.dropdown_drawings);

    Widgets.dropdown_transformations = gtk_drop_down_new_from_strings(dropdown_content_transformations);
    gtk_box_append(GTK_BOX(Widgets.option_box), Widgets.dropdown_transformations);
 
    Widgets.dropdown_croppings = gtk_drop_down_new_from_strings(dropdown_content_croppings);
    gtk_box_append(GTK_BOX(Widgets.option_box), Widgets.dropdown_croppings);

    Widgets.entry_buffer = gtk_entry_buffer_new(NULL, -1);
    Widgets.main_input = gtk_entry_new_with_buffer(Widgets.entry_buffer);
    gtk_entry_set_placeholder_text(GTK_ENTRY(Widgets.main_input), "Transformation values here...");
    gtk_box_append(GTK_BOX(Widgets.option_box), Widgets.main_input);

    /*https://stackoverflow.com/questions/73334375/how-to-pass-2-or-more-gtk-widget-to-callback-function*/
    Widgets.label = gtk_label_new("Debug informations here...");
    gtk_box_append(GTK_BOX(Widgets.option_box), Widgets.label);

    g_signal_connect(Widgets.dropdown_algorithms, "notify::selected", G_CALLBACK(algorithms_execution), NULL);
    g_signal_connect(Widgets.dropdown_drawings, "notify::selected", G_CALLBACK(drawings_execution), GTK_DRAWING_AREA(Widgets.drawing_area));
    g_signal_connect(Widgets.dropdown_transformations, "notify::selected", G_CALLBACK(transformation_execution), Widgets.entry_buffer);
    g_signal_connect(Widgets.dropdown_croppings, "notify::selected", G_CALLBACK(cropping_selection), NULL);
    gtk_window_present(GTK_WINDOW(Widgets.window));

}

int main(int    argc,
         char** argv)
{
    GtkApplication *app;
    int status;
    arr_points = array_create(MAX_POINTS);
    arr_lines = array_create(MAX_POINTS);
    arr_polygons = array_create(MAX_POINTS);
    arr_circumferences = array_create(MAX_POINTS);
    arr_clips = array_create(MAX_POINTS);

    app = gtk_application_new("GC.Thiago", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);

    return status;
}