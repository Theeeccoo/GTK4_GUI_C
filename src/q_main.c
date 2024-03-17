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

// #define MAX_POINTS 10

static cairo_surface_t *surface = NULL;
static int algh = 0;
static int cropp = 0;

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
    // g_print("%lf %lf\n", x, y);
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
    g_print("1 X %f, Y %f\n", x, y);
    // Redefining center point to be exactly the center of canvas. In canvas 0,0 is in the top-left corner.
    int center_x = 0,
        center_y = 0; 
    gtk_widget_get_size_request(Widgets.drawing_area, &center_x, &center_y);
    x = x - (double)(center_x / 2);
    y = (double)(center_y / 2) - y;
    struct point *p = point_create(x, y);
    array_set(arr_points, array_get_curr_num(arr_points), p);

    g_print(" X %f, Y %f\n", point_x_coord(p), point_y_coord(p));

    // If cropping algorithm is NOT selected, define point color to black, otherwise, red
    if ( !cropp ) point_define_color(p, 0.0, 0.0, 0.0);
    else          point_define_color(p, 255.0, 0.0, 0.0);

    cairo_t *cr;

    cr = cairo_create(surface);
    draw_brush(area, cr, x, y, color_get_colors(point_color(p)));
    draw_text(area, cr, p);
    cairo_destroy(cr);
}

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
    g_print("Drawing: %d!\n", dropdown_selected);
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

// TODO Isso vai pra Utils
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

// TODO Isso vai pra Utils ( Tá com erro, tem que arrumar, pro código agora não será utilizado )
double my_strtod(char* content)
{
    double result = 0.0f;
    int num_size = 0,
        mantissa_size = 0,
        iterator = 0,
        positive = 1;

    if ( content[iterator] == '-' ) { positive = 0; iterator++; } 
    else if ( content[iterator] == '+') iterator++;

    
    while ( content[iterator++] != '.' )  
    {
        // Making sure that what is read is actually a number.
        assert(content[iterator] > '0' && content[iterator] < '9');
        num_size++;
    }

    while ( iterator != strlen(content) ) 
    {
        assert(content[iterator] > '0' && content[iterator] < '9');
        mantissa_size++;
    }

    
    for ( int i = 0; i < num_size ; i++ )
    {
        // Since we are sure that everything is a number, we can force a typecasting
        int curr_value = ((int) content[i]) - '0';
        double power = pow(10.0f, num_size - i - 1);
        result += curr_value * power;
    }

    for ( int i = 0; i < mantissa_size; i++ )
    {
        // Since we are sure that everything is a number, we can force a typecasting
        int curr_value = ((int) content[i]) - '0';
        double power = pow(10.0f, i + 1);
        result += curr_value / power;
    }

    if ( positive == 0 ) result *= -1;
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
    // Rotation. Should be informed as: 'X' - Where X is an integer rotation degree value (pos or neg).
    else if ( transf_id == 2 )
    {
        if ( strlen(content) > 5 )
        {
            gtk_label_set_label(GTK_LABEL(Widgets.label), "WARNING: Rotation template is: 'INT'. Please, reformulate your input.");
            return NULL;
        }

        double ang = 0.0f;
        first = read_from_until(content, 'd', &controller);
        if ( first == NULL ) return NULL;

        ang = strtod(first, &foo);
        g_print("Anb %f\n", ang);
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

Bool xyreflection()
{
    int aux[MAX_POINTS],
        cont = 0;
    for (int i = 0; i < MAX_POINTS; i++ ) aux[i] = -2;

    clear_surface(0);
    gtk_widget_queue_draw(Widgets.drawing_area);
    cairo_t *cr;
    cr = cairo_create(surface);

    // Lines.
    for ( int i = 0; i < array_get_curr_num(arr_lines); i++ )
    {
        line_tt foo = array_get(arr_lines, i);

        point_tt *points = line_get_points(foo);
        int line_algh = line_get_algh(foo);

        for ( int j = 0; j < 2; j++ )
        {
            aux[cont++] = point_id(points[j]);
            double new_x = -point_x_coord(points[j]),
                   new_y = point_y_coord(points[j]);
            point_set_coord(points[j], new_x, new_y);
            draw_brush(Widgets.drawing_area, cr, new_x, new_y, color_get_colors(point_color(points[j])));
            draw_text(Widgets.drawing_area, cr, points[j]);
        }

        if ( line_algh == 1 ) DDA(points[0], points[1], Widgets.drawing_area);
        else if ( line_algh == 2 ) Bresenham(points[0], points[1], Widgets.drawing_area);
    }

    // Polygons.
    for ( int i = 0; i < array_get_curr_num(arr_polygons); i++ )
    {
        polygon_tt foo = array_get(arr_polygons, i);

        array_tt p_points = polygon_get_points(foo);
        int line_algh = polygon_get_algh(foo);

        for ( int j = 0; j < array_get_curr_num(p_points) ; j++ )
        {   
            struct point *p = array_get(p_points, j);
            aux[cont++] = point_id(p);
            double new_x = -point_x_coord(array_get(p_points, j)),
                   new_y = -point_y_coord(array_get(p_points, j));
            point_set_coord(p, new_x, new_y);
            draw_brush(Widgets.drawing_area, cr, new_x, new_y, color_get_colors(point_color(p)));
            draw_text(Widgets.drawing_area, cr, p);
        }

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
    
    // Circumference.
    for ( int i = 0; i < array_get_curr_num(arr_circumferences); i++ )
    {
        circumference_tt foo = array_get(arr_circumferences, i);

        point_tt *points = circumference_get_points(foo);

        for ( int j = 0; j < 2; j++ )
        {
            aux[cont++] = point_id(points[j]);
            double new_x = -point_x_coord(points[j]),
                   new_y = -point_y_coord(points[j]);
            point_set_coord(points[j], new_x, new_y);
            draw_brush(Widgets.drawing_area, cr, new_x, new_y, color_get_colors(point_color(points[j])));
            draw_text(Widgets.drawing_area, cr, points[j]);
        }

        calculate_circumference_points(Widgets.drawing_area, foo);
    }

    // Drawing all points that aren't part of an object
    Bool drawn = false;
    for ( int i = 0; i < array_get_curr_num(arr_points); i++ )
    {
        drawn = false;
        point_tt p = array_get(arr_points, i);
        int id = point_id(p);

        for ( int j = 0; j < MAX_POINTS; j++ )
        {   
            if ( id == aux[j] ) drawn = true;
        }

        if ( !drawn ) 
        {
            draw_brush(Widgets.drawing_area, cr, point_x_coord(p), point_y_coord(p), color_get_colors(point_color(p)));
            draw_text(Widgets.drawing_area, cr, p);
        }
    }
    cairo_destroy(cr);


    return True;
}

Bool yreflection()
{
    int aux[MAX_POINTS],
        cont = 0;
    for (int i = 0; i < MAX_POINTS; i++ ) aux[i] = -2;

    clear_surface(0);
    gtk_widget_queue_draw(Widgets.drawing_area);
    cairo_t *cr;
    cr = cairo_create(surface);
    
    // Lines.
    for ( int i = 0; i < array_get_curr_num(arr_lines); i++ )
    {
        line_tt foo = array_get(arr_lines, i);

        point_tt *points = line_get_points(foo);
        int line_algh = line_get_algh(foo);

        for ( int j = 0; j < 2; j++ )
        {
            aux[cont++] = point_id(points[j]);
            double new_x = -point_x_coord(points[j]),
                   new_y = point_y_coord(points[j]);
            point_set_coord(points[j], new_x, new_y);
            draw_brush(Widgets.drawing_area, cr, new_x, new_y, color_get_colors(point_color(points[j])));
            draw_text(Widgets.drawing_area, cr, points[j]);
        }


        if ( line_algh == 1 ) DDA(points[0], points[1], Widgets.drawing_area);
        else if ( line_algh == 2 ) Bresenham(points[0], points[1], Widgets.drawing_area);
    }

    // Polygons.
    for ( int i = 0; i < array_get_curr_num(arr_polygons); i++ )
    {
        polygon_tt foo = array_get(arr_polygons, i);

        array_tt p_points = polygon_get_points(foo);
        int line_algh = polygon_get_algh(foo);

        for ( int j = 0; j < array_get_curr_num(p_points) ; j++ )
        {   
            struct point *p = array_get(p_points, j);
            aux[cont++] = point_id(p);
            double new_x = -point_x_coord(array_get(p_points, j)),
                   new_y = point_y_coord(array_get(p_points, j));
            point_set_coord(p, new_x, new_y);
            draw_brush(Widgets.drawing_area, cr, new_x, new_y, color_get_colors(point_color(p)));
            draw_text(Widgets.drawing_area, cr, p);
        }

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

    // Circumference.
    for ( int i = 0; i < array_get_curr_num(arr_circumferences); i++ )
    {
        circumference_tt foo = array_get(arr_circumferences, i);

        point_tt *points = circumference_get_points(foo);

        for ( int j = 0; j < 2; j++ )
        {
            aux[cont++] = point_id(points[j]);
            double new_x = -point_x_coord(points[j]),
                   new_y = point_y_coord(points[j]);
            point_set_coord(points[j], new_x, new_y);
            draw_brush(Widgets.drawing_area, cr, new_x, new_y, color_get_colors(point_color(points[j])));
            draw_text(Widgets.drawing_area, cr, points[j]);
        }

        calculate_circumference_points(Widgets.drawing_area, foo);
    }


    // Drawing all points that aren't part of an object
    Bool drawn = false;
    for ( int i = 0; i < array_get_curr_num(arr_points); i++ )
    {
        drawn = false;
        point_tt p = array_get(arr_points, i);
        int id = point_id(p);

        for ( int j = 0; j < MAX_POINTS; j++ )
        {   
            if ( id == aux[j] ) drawn = true;
        }

        if ( !drawn ) 
        {
            draw_brush(Widgets.drawing_area, cr, point_x_coord(p), point_y_coord(p), color_get_colors(point_color(p)));
            draw_text(Widgets.drawing_area, cr, p);
        }
    }

    cairo_destroy(cr);
    return True;
}

Bool xreflection()
{
    int aux[MAX_POINTS],
        cont = 0;
    for (int i = 0; i < MAX_POINTS; i++ ) aux[i] = -2;

    clear_surface(0);
    gtk_widget_queue_draw(Widgets.drawing_area);
    cairo_t *cr;
    cr = cairo_create(surface);
    
    // Lines.
    for ( int i = 0; i < array_get_curr_num(arr_lines); i++ )
    {
        
        line_tt foo = array_get(arr_lines, i);

        point_tt *points = line_get_points(foo);
        int line_algh = line_get_algh(foo);

        for ( int j = 0; j < 2; j++ )
        {
            aux[cont++] = point_id(points[j]);
            double new_x = point_x_coord(points[j]),
                   new_y = -point_y_coord(points[j]);
            point_set_coord(points[j], new_x, new_y);
            draw_brush(Widgets.drawing_area, cr, new_x, new_y, color_get_colors(point_color(points[j])));
            draw_text(Widgets.drawing_area, cr, points[j]);
        }


        if ( line_algh == 1 ) DDA(points[0], points[1], Widgets.drawing_area);
        else if ( line_algh == 2 ) Bresenham(points[0], points[1], Widgets.drawing_area);
    }

    // Polygons
    for ( int i = 0; i < array_get_curr_num(arr_polygons); i++ )
    {
        polygon_tt foo = array_get(arr_polygons, i);

        array_tt p_points = polygon_get_points(foo);
        int line_algh = polygon_get_algh(foo);

        for ( int j = 0; j < array_get_curr_num(p_points) ; j++ )
        {   
            struct point *p = array_get(p_points, j);
            aux[cont++] = point_id(p);
            double new_x = point_x_coord(array_get(p_points, j)),
                   new_y = -point_y_coord(array_get(p_points, j));
            point_set_coord(p, new_x, new_y);
            draw_brush(Widgets.drawing_area, cr, new_x, new_y, color_get_colors(point_color(p)));
            draw_text(Widgets.drawing_area, cr, p);
        }

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

    // Circumference.
    for ( int i = 0; i < array_get_curr_num(arr_circumferences); i++ )
    {
        circumference_tt foo = array_get(arr_circumferences, i);

        point_tt *points = circumference_get_points(foo);

        for ( int j = 0; j < 2; j++ )
        {
            aux[cont++] = point_id(points[j]);
            double new_x = point_x_coord(points[j]),
                   new_y = -point_y_coord(points[j]);
            point_set_coord(points[j], new_x, new_y);
            draw_brush(Widgets.drawing_area, cr, new_x, new_y, color_get_colors(point_color(points[j])));
            draw_text(Widgets.drawing_area, cr, points[j]);
        }

        calculate_circumference_points(Widgets.drawing_area, foo);
    }

    // Drawing all points that aren't part of an object
    Bool drawn = false;
    for ( int i = 0; i < array_get_curr_num(arr_points); i++ )
    {
        drawn = false;
        point_tt p = array_get(arr_points, i);
        int id = point_id(p);

        for ( int j = 0; j < MAX_POINTS; j++ )
        {   
            if ( id == aux[j] ) drawn = true;
        }

        if ( !drawn ) 
        {
            draw_brush(Widgets.drawing_area, cr, point_x_coord(p), point_y_coord(p), color_get_colors(point_color(p)));
            draw_text(Widgets.drawing_area, cr, p);
        }
    }
    cairo_destroy(cr);
    return True;
}

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
    int aux[MAX_POINTS],
        cont = 0;
    for (int i = 0; i < MAX_POINTS; i++ ) aux[i] = -2;   

    // Pinning first point always
    clear_surface(0);
    gtk_widget_queue_draw(Widgets.drawing_area);
    cairo_t *cr;
    cr = cairo_create(surface);
    g_print("%lf Rotation\n", rotation[0]);

    // Lines.
    for ( int i = 0; i < array_get_curr_num(arr_lines); i++ )
    {
        line_tt foo = array_get(arr_lines, i);

        point_tt *points = line_get_points(foo);
        int line_algh = line_get_algh(foo);

        // TODO Probably this can be anexed in a new function (V)
        double pinned_x = point_x_coord(points[0]),
               pinned_y = point_y_coord(points[0]);
        draw_brush(Widgets.drawing_area, cr, pinned_x, pinned_y, color_get_colors(point_color(points[0])));
        draw_text(Widgets.drawing_area, cr, points[0]);
        aux[cont++] = point_id(points[0]);

        for ( int j = 1; j < 2; j++ )
        {
            aux[cont++] = point_id(points[j]);
            double new_x = ( (point_x_coord(points[j]) - pinned_x) * cos(rotation[0])) - ( (point_y_coord(points[j]) - pinned_y) * sin(rotation[0])),
                   new_y = ( (point_x_coord(points[j]) - pinned_x) * sin(rotation[0])) + ( (point_y_coord(points[j]) - pinned_y) * cos(rotation[0]));
            point_set_coord(points[j], new_x + pinned_x, new_y + pinned_y);
            draw_brush(Widgets.drawing_area, cr, new_x + pinned_x, new_y + pinned_y, color_get_colors(point_color(points[j])));
            draw_text(Widgets.drawing_area, cr, points[j]);
        }
        

        if ( line_algh == 1 ) DDA(points[0], points[1], Widgets.drawing_area);
        else if ( line_algh == 2 ) Bresenham(points[0], points[1], Widgets.drawing_area);
    }

    // Polygons
    for ( int i = 0; i < array_get_curr_num(arr_polygons); i++ )
    {
        polygon_tt foo = array_get(arr_polygons, i);

        array_tt p_points = polygon_get_points(foo);
        int line_algh = polygon_get_algh(foo);

        point_tt p_pinned = array_get(p_points, 0);
        double pinned_x = point_x_coord(p_pinned),
               pinned_y = point_y_coord(p_pinned);
        draw_brush(Widgets.drawing_area, cr, pinned_x, pinned_y, color_get_colors(point_color(p_pinned)));
        draw_text(Widgets.drawing_area, cr, p_pinned);
        aux[cont++] = point_id(p_pinned);

        for ( int j = 1; j < array_get_curr_num(p_points) ; j++ )
        {   
            struct point *p = array_get(p_points, j);
            aux[cont++] = point_id(p);
            double new_x = ( (point_x_coord(p) - pinned_x) * cos(rotation[0])) - ( (point_y_coord(p) - pinned_y) * sin(rotation[0])),
                   new_y = ( (point_x_coord(p) - pinned_x) * sin(rotation[0])) + ( (point_y_coord(p) - pinned_y) * cos(rotation[0]));
            point_set_coord(p, new_x + pinned_x, new_y + pinned_y);
            draw_brush(Widgets.drawing_area, cr, new_x + pinned_x, new_y + pinned_y, color_get_colors(point_color(p)));
            draw_text(Widgets.drawing_area, cr, p);
        }

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

    // Circumference.
    for ( int i = 0; i < array_get_curr_num(arr_circumferences); i++ )
    {
        circumference_tt foo = array_get(arr_circumferences, i);

        point_tt *points = circumference_get_points(foo);

        for ( int j = 0; j < 2; j++ )
        {
            draw_brush(Widgets.drawing_area, cr, point_x_coord(points[j]), point_y_coord(points[j]), color_get_colors(point_color(points[j])));
            draw_text(Widgets.drawing_area, cr, points[j]);
        }

        calculate_circumference_points(Widgets.drawing_area, foo);
    }

    // Drawing all points that aren't part of an object
    Bool drawn = false;
    for ( int i = 0; i < array_get_curr_num(arr_points); i++ )
    {
        drawn = false;
        point_tt p = array_get(arr_points, i);
        int id = point_id(p);

        for ( int j = 0; j < MAX_POINTS; j++ )
        {   
            if ( id == aux[j] ) drawn = true;
        }

        if ( !drawn ) 
        {
            draw_brush(Widgets.drawing_area, cr, point_x_coord(p), point_y_coord(p), color_get_colors(point_color(p)));
            draw_text(Widgets.drawing_area, cr, p);
        }
    }
    g_print("%lf 22Rotation\n", rotation[0]);

    cairo_destroy(cr);
    return True;
}

Bool scale(char *content, 
           int   transf_id)
{
    // Sanity Check
    if ( strlen(content) == 0 ) 
    {
        gtk_label_set_label(GTK_LABEL(Widgets.label), "WARNING: You must provide paramethers to this transformation.");
        return False;
    }

    // When Scaling, whenever a value is negative, it means to SHRINK "the points". Positive values means to increase
    double *scale = NULL;
    if ( (scale = check_content(content, transf_id)) == NULL ) return False;

    // Pinning first point always. If not Pinned, scale will also be a translation
    int aux[MAX_POINTS],
        cont = 0;
    for (int i = 0; i < MAX_POINTS; i++ ) aux[i] = -2;

    clear_surface(0);
    gtk_widget_queue_draw(Widgets.drawing_area);
    cairo_t *cr;
    cr = cairo_create(surface);

    // Lines
    for ( int i = 0; i < array_get_curr_num(arr_lines); i++ )
    {
        line_tt foo = array_get(arr_lines, i);
        point_tt *points = line_get_points(foo);
        int line_algh = line_get_algh(foo);

        for ( int j = 0; j < 2; j++ )
        {
            aux[cont++] = point_id(points[j]);

            double new_x = point_x_coord(points[j]),
                   new_y = point_y_coord(points[j]);
            int line_algh = line_get_algh(foo);
            if ( scale[0] < 0 ) new_x /= abs(scale[0]);
            else new_x *= scale[0];
            if ( scale[1] < 0 ) new_y /= abs(scale[1]);
            else new_y *= scale[1]; 

            point_set_coord(points[j], new_x, new_y);
            draw_brush(Widgets.drawing_area, cr, new_x, new_y, color_get_colors(point_color(points[j])));
            draw_text(Widgets.drawing_area, cr, points[j]);
        }
        

        if ( line_algh == 1 ) DDA(points[0], points[1], Widgets.drawing_area);
        else if ( line_algh == 2 ) Bresenham(points[0], points[1], Widgets.drawing_area);

    }

    // Polygons
    for ( int i = 0; i < array_get_curr_num(arr_polygons); i++ )
    {
        polygon_tt foo = array_get(arr_polygons, i);

        array_tt p_points = polygon_get_points(foo);
        int line_algh = polygon_get_algh(foo);

        for ( int j = 0; j < array_get_curr_num(p_points) ; j++ )
        {   
            struct point *p = array_get(p_points, j);
            aux[cont++] = point_id(p);
            double new_x = point_x_coord(array_get(p_points, j)),
                   new_y = point_y_coord(array_get(p_points, j));

            if ( scale[0] < 0 ) new_x /= abs(scale[0]);
            else new_x *= scale[0];
            if ( scale[1] < 0 ) new_y /= abs(scale[1]);
            else new_y *= scale[1]; 
            point_set_coord(p, new_x, new_y);
            draw_brush(Widgets.drawing_area, cr, new_x, new_y, color_get_colors(point_color(p)));
            draw_text(Widgets.drawing_area, cr, p);
        }

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

    // Circumference.
    for ( int i = 0; i < array_get_curr_num(arr_circumferences); i++ )
    {
        circumference_tt foo = array_get(arr_circumferences, i);

        point_tt *points = circumference_get_points(foo);

        for ( int j = 0; j < 2; j++ )
        {
            aux[cont++] = point_id(points[j]);
            double new_x = point_x_coord(points[j]),
                   new_y = point_y_coord(points[j]);

            if ( scale[0] < 0 ) new_x /= abs(scale[0]);
            else new_x *= scale[0];
            if ( scale[1] < 0 ) new_y /= abs(scale[1]);
            else new_y *= scale[1]; 

            point_set_coord(points[j], new_x, new_y);
            draw_brush(Widgets.drawing_area, cr, new_x, new_y, color_get_colors(point_color(points[j])));
            draw_text(Widgets.drawing_area, cr, points[j]);
        }

        calculate_circumference_points(Widgets.drawing_area, foo);
    }
    // Drawing all points that aren't part of an object
    Bool drawn = false;
    for ( int i = 0; i < array_get_curr_num(arr_points); i++ )
    {
        drawn = false;
        point_tt p = array_get(arr_points, i);
        int id = point_id(p);

        for ( int j = 0; j < MAX_POINTS; j++ )
        {   
            if ( id == aux[j] ) drawn = true;
        }

        if ( !drawn ) 
        {
            draw_brush(Widgets.drawing_area, cr, point_x_coord(p), point_y_coord(p), color_get_colors(point_color(p)));
            draw_text(Widgets.drawing_area, cr, p);
        }
    }

    g_print("Scale: %f %f\n", scale[0], scale[1]);
    cairo_destroy(cr);

    free(scale);
    return True;


}

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

    int aux[MAX_POINTS],
        cont = 0;
    for (int i = 0; i < MAX_POINTS; i++ ) aux[i] = -2;   
    
    clear_surface(0);
    gtk_widget_queue_draw(Widgets.drawing_area);
    cairo_t *cr;
    cr = cairo_create(surface);
    
    // Lines
    for ( int i = 0; i < array_get_curr_num(arr_lines); i++ )
    {
        line_tt foo = array_get(arr_lines, i);

        point_tt *points = line_get_points(foo);
        int line_algh = line_get_algh(foo);

        // TODO Probably this can be anexed in a new function (V)
        for ( int j = 0; j < 2; j++ )
        {
            aux[cont++] = point_id(points[j]);
            double new_x = point_x_coord(points[j]) + translation[0],
                   new_y = point_y_coord(points[j]) + translation[1];
            point_set_coord(points[j], new_x, new_y);
            draw_brush(Widgets.drawing_area, cr, new_x, new_y, color_get_colors(point_color(points[j])));
            draw_text(Widgets.drawing_area, cr, points[j]);
        }
        

        if ( line_algh == 1 ) DDA(points[0], points[1], Widgets.drawing_area);
        else if ( line_algh == 2 ) Bresenham(points[0], points[1], Widgets.drawing_area);
    }

    // Polygons
    for ( int i = 0; i < array_get_curr_num(arr_polygons); i++ )
    {
        polygon_tt foo = array_get(arr_polygons, i);

        array_tt p_points = polygon_get_points(foo);
        int line_algh = polygon_get_algh(foo);

        for ( int j = 0; j < array_get_curr_num(p_points) ; j++ )
        {   
            struct point *p = array_get(p_points, j);
            aux[cont++] = point_id(p);
            double new_x = point_x_coord(p) + translation[0],
                   new_y = point_y_coord(p) + translation[1];
            point_set_coord(p, new_x, new_y);
            draw_brush(Widgets.drawing_area, cr, new_x, new_y, color_get_colors(point_color(p)));
            draw_text(Widgets.drawing_area, cr, p);
        }

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

    // Circumference.
    for ( int i = 0; i < array_get_curr_num(arr_circumferences); i++ )
    {
        circumference_tt foo = array_get(arr_circumferences, i);

        point_tt *points = circumference_get_points(foo);

        for ( int j = 0; j < 2; j++ )
        {
            aux[cont++] = point_id(points[j]);
            double new_x = point_x_coord(points[j]) + translation[0],
                   new_y = point_y_coord(points[j]) + translation[1];
            point_set_coord(points[j], new_x, new_y);
            draw_brush(Widgets.drawing_area, cr, new_x, new_y, color_get_colors(point_color(points[j])));
            draw_text(Widgets.drawing_area, cr, points[j]);
        }

        calculate_circumference_points(Widgets.drawing_area, foo);
    }

    // TODO assim como em todas as outras transformadas, checar se, quando a transformada for feita, o objeto tá dentro da área de recorte

    // Drawing all points that aren't part of an object
    Bool drawn = false;
    for ( int i = 0; i < array_get_curr_num(arr_points); i++ )
    {
        drawn = false;
        point_tt p = array_get(arr_points, i);
        int id = point_id(p);

        for ( int j = 0; j < MAX_POINTS; j++ )
        {   
            if ( id == aux[j] ) drawn = true;
        }

        if ( !drawn ) 
        {
            draw_brush(Widgets.drawing_area, cr, point_x_coord(p), point_y_coord(p), color_get_colors(point_color(p)));
            draw_text(Widgets.drawing_area, cr, p);
        }
    }
    cairo_destroy(cr);
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

    g_print("Hello World %s %d!\n", content, dropdown_selected);
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

    cropp = (dropdown_selected == 0) ? 0 : 1;
}   

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
    const char *dropdown_content_croppings[5] = {"Cropping Algorithms\0", "Cohen-Sutherland\0", "Liang-Barsky\0", "Clear"};

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

    app = gtk_application_new("GC.Thiago", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);

    return status;
}