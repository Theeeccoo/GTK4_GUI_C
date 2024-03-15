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

#include "point.h"
#include "array.h"
#include "line.h"

#define MAX_POINTS 10

static cairo_surface_t *surface = NULL;
static GtkWidget *label = NULL;
static int algh = 0;
static int cropp = 0;

/**
 * @brief Current clicked Points.
*/
static array_tt arr_points;

/**
 * @brief Current drawn Lines.
*/
static array_tt arr_lines;

/**
 * @brief Removes all points drawn in canvas.
*/
static void clear_surface(void)
{
    cairo_t *cr = cairo_create(surface);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    cairo_destroy(cr);

    // Recreating array that contains all points
    array_destroy(arr_points);
    arr_points = array_create(MAX_POINTS);
    array_destroy(arr_lines);
    arr_lines = array_create(MAX_POINTS);
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



        clear_surface();
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
    double x = point_x_coord(p),
           y = point_y_coord(p);
    char *result = create_string(x, y);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 12.0);

    cairo_move_to(cr, x - 34, y + 15 );
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
    // g_print("%lf %lf\n", x, y);
    cairo_set_source_rgb(cr, c[0], c[1], c[2]);
    cairo_rectangle(cr, x - 3, y - 3, 6, 6);
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
    struct point *p = point_create(x, y);
    array_set(arr_points, array_get_curr_num(arr_points), p);

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
    clear_surface();
    gtk_widget_queue_draw(area);
}

/**
 * @brief (CALL_BACK) Function called whenever user closes main window
*/
static void close_window(void)
{
    array_destroy(arr_points);
    array_destroy(arr_lines);
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

/**
 * @brief Creates and draws Lines with previously specified Drawing Algorithm.
 * 
 * 
 * @param area Drawing Area
 * 
 * @returns True if code was correctly executed, False otherwise
*/
Bool Line(GtkWidget *area)
{
    // If no algorithm selected, "Error".
    if ( algh == 0 )
    {
        gtk_label_set_label(GTK_LABEL(label), "WARNING: You must provide the drawing algorithm.");
        return False;
    }

    int controller = array_get_curr_num(arr_lines) * 2,
        size_p = array_get_curr_num(arr_points);
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
            controller++;
            struct line *new_line = line_create(pInit, pFinal, algh);
            array_set(arr_lines, array_get_curr_num(arr_lines), new_line);
            if ( algh == 1 ) DDA(pInit, pFinal, area);
            else if ( algh == 2 ) Bresenham(pInit, pFinal, area);
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
    gtk_label_set_label(GTK_LABEL(label), result);
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

    GtkWidget *drawing_area = GTK_WIDGET(user_data);
    g_print("Drawing: %d!\n", dropdown_selected);
    switch (dropdown_selected)
    {
        case 1:
            clock_t t = clock();
            Bool cntrl = Line(drawing_area);
            t = clock() - t;
            if ( cntrl )
                write_execution_time(t);
            break;
        case 2:
            // Polygon();
            break;
        case 3:
            // Circumference();
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
    while ( content[*pos] != stop )
    {
        // Ensuring that only numbers will be in our template
        if ( (content[*pos] < '0' || content[*pos] > '9') && content[*pos] != '.' && content[*pos] != '-' && content[*pos] != '+')
        {
            gtk_label_set_label(GTK_LABEL(label), "WARNING: Translation template is: '(X,Y)'. Please, reformulate your input.");
            return NULL;
        }
        result[controller++] = content[(*pos)++];
    }
    result[controller] = '\0';
    return result;
}

double* check_content(char* content, 
                      int   transf_id)
{
    char *first  = (char*) malloc(sizeof(char) * 100),
         *second = (char*) malloc(sizeof(char) * 100),
         *foo = (char*) malloc(sizeof(char) * 100);
    int controller = 1;
    double *values = (double*) malloc(sizeof(double) * 3);
    // Translation. Should be informed as: (X,Y) - Where X and Y are the values that will be incremented to current points.
    if ( transf_id == 1 )
    {
        if ( strlen(content) < 5 )
        {
            gtk_label_set_label(GTK_LABEL(label), "WARNING: Translation template is: '(X,Y)'. Please, reformulate your input.");
            return NULL;
        }
        double x = 0.0f,
               y = 0.0f;

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

    // House Keeping
    free(first);
    free(second);
    free(foo);
}

Bool translation(char *content,
                 int   transf_id)
{
    if ( strlen(content) == 0 ) 
    {
        gtk_label_set_label(GTK_LABEL(label), "WARNING: You must provide paramethers to a transformation.");
        return False;
    }

    double *translation = NULL;
    if ( (translation = check_content(content, transf_id)) == NULL ) return False;

    g_print("Translation: %f %f\n", translation[0], translation[1]);

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

    switch (dropdown_selected)
    {
        case 1:
            clock_t t = clock();
            Bool cntrl = translation(content, dropdown_selected);
            t = clock() - t;
            if ( cntrl )
                write_execution_time(t);
            break;
        case 2:
            // rotation(content);
            break;
        case 3:
            // scale(content);
            break;
        case 4:
            // xreflection();
            break;
        case 5: 
            // yreflection();
            break;
        case 6:
            // xyreflection();
            break;
        default:
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
                   *main_input;
    GtkGesture     *drag, 
                   *press;
    GtkEntryBuffer *entry_buffer;


    const char *dropdown_content_algorithms[4] = {"Line Algorithms\0", "DDA\0", "Bresenham\0"};
    const char *dropdown_content_drawings[5] = {"Draw\0", "Line\0", "Polygon\0", "Circumference\0"};
    const char *dropdown_content_transformations[8] = {"Geometric Transformations\0", "Translate\0", "Rotate\0", "Scale\0", "X Reflection\0", "Y Reflection\0", "XY Reflection\0"};
    const char *dropdown_content_croppings[4] = {"Cropping Algorithms\0", "Cohen-Sutherland\0", "Liang-Barsky\0"};

    int width,
        height;

    user_monitor_info(&width, &height);

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "GC Project");

    g_signal_connect(window, "destroy", G_CALLBACK(close_window), NULL);

    
    main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_window_set_child(GTK_WINDOW(window), main_box);

    frame_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_append(GTK_BOX(main_box), frame_box);

    frame = gtk_frame_new(NULL);
    gtk_box_append(GTK_BOX(frame_box), frame);

    drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area, width, ((height * 91) / 100));
    gtk_frame_set_child(GTK_FRAME(frame), drawing_area);

    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(drawing_area), draw_cb, NULL, NULL);
    g_signal_connect_after(drawing_area, "resize", G_CALLBACK(resize_cb), NULL);

    drag = gtk_gesture_drag_new();
    gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(drag), GDK_BUTTON_PRIMARY);
    gtk_widget_add_controller(drawing_area, GTK_EVENT_CONTROLLER(drag));
    g_signal_connect(drag, "drag-begin", G_CALLBACK(draw), drawing_area); 

    press = gtk_gesture_click_new();
    gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(press), GDK_BUTTON_SECONDARY);
    gtk_widget_add_controller(drawing_area, GTK_EVENT_CONTROLLER(press));

    g_signal_connect(press, "pressed", G_CALLBACK(clean), drawing_area);
    
    option_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_append(GTK_BOX(main_box), option_box);
    gtk_box_set_spacing(GTK_BOX(option_box), 5);

    dropdown_algorithms = gtk_drop_down_new_from_strings(dropdown_content_algorithms);
    gtk_box_append(GTK_BOX(option_box), dropdown_algorithms);

    dropdown_drawings = gtk_drop_down_new_from_strings(dropdown_content_drawings);
    gtk_box_append(GTK_BOX(option_box), dropdown_drawings);

    dropdown_transformations = gtk_drop_down_new_from_strings(dropdown_content_transformations);
    gtk_box_append(GTK_BOX(option_box), dropdown_transformations);
 
    dropdown_croppings = gtk_drop_down_new_from_strings(dropdown_content_croppings);
    gtk_box_append(GTK_BOX(option_box), dropdown_croppings);

    entry_buffer = gtk_entry_buffer_new(NULL, -1);
    main_input = gtk_entry_new_with_buffer(entry_buffer);
    gtk_entry_set_placeholder_text(GTK_ENTRY(main_input), "Transformation values here...");
    gtk_box_append(GTK_BOX(option_box), main_input);

    /*https://stackoverflow.com/questions/73334375/how-to-pass-2-or-more-gtk-widget-to-callback-function*/
    label = gtk_label_new("Debug informations here...");
    gtk_box_append(GTK_BOX(option_box), label);

    g_signal_connect(dropdown_algorithms, "notify::selected", G_CALLBACK(algorithms_execution), NULL);
    g_signal_connect(dropdown_drawings, "notify::selected", G_CALLBACK(drawings_execution), GTK_DRAWING_AREA(drawing_area));
    g_signal_connect(dropdown_transformations, "notify::selected", G_CALLBACK(transformation_execution), entry_buffer);
    g_signal_connect(dropdown_croppings, "notify::selected", G_CALLBACK(cropping_selection), NULL);
    gtk_window_present(GTK_WINDOW(window));

}

int main(int    argc,
         char** argv)
{
    GtkApplication *app;
    int status;
    arr_points = array_create(MAX_POINTS);
    arr_lines = array_create(MAX_POINTS);

    app = gtk_application_new("GC.Thiago", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);

    return status;
}