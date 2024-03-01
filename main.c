#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <gtk/gtk.h>
#include <cairo.h>

// Used to make the app fit correctly, proportionately, into users window
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>

#define MAX_POINTS 10

static cairo_surface_t *surface = NULL;

/**
 * @brief Points structure data.
*/
struct {
    int initialized; /** << Initialized?                  */
    int       count; /** << Number of drawn points.       */
    double  *coordx; /** << X coordinate of drawn points. */
    double  *coordy; /** << Y coordinate of drawn points. */
} points = { 0, 0, NULL, NULL };

/**
 * @brief Initializes the Points structure.
*/
void points_init(void)
{
    /* Sanity test - Already initialized */
    if ( points.initialized ) return;

    points.initialized = 1;
    points.coordx = (double*) malloc (sizeof(double) * MAX_POINTS);
    points.coordy = (double*) malloc (sizeof(double) * MAX_POINTS);

    // TODO This initial value might be wrong if we change the 0,0 to be in the exact middle of canvas
    for ( int i = 0; i < MAX_POINTS; i++ )
    {
        points.coordx[i] = points.coordy[i] = -1;
    }
}

/**
 * @brief Finilizes the Points structure.
*/
void points_destroy(void)
{
    /* Sanity test - Not initialized */
    if ( !points.initialized ) return;

    points.initialized = 0;
    points.count = 0;
    free(points.coordx);
    free(points.coordy);
}


/**
 * @brief Adds a new Coordinate (X,Y) into Points
 * 
 * @param x X coordinate in canvas.
 * @param y Y coordinate in canvas.
 * 
 * @returns Number of current total points in Points.
*/
int points_add(double x, 
               double y)
{
    /* Sanity Test */
    assert( points.count < MAX_POINTS );

    points.coordx[points.count] = x;
    points.coordy[points.count++] = y;

    return points.count;
}

/**
 * @brief Removes all points in Points0
 * 
 * @returns Number of current total points in Points.
*/
int points_clear(void)
{
    /* Sanity Test */
    assert( points.count > 0 );

    for ( int i = 0; i < points.count; i++ )
    {
        points.coordx[i] = points.coordy[i] = -1;
    }
    points.count = 0;

    return points.count;
}

/**
 * @brief Removes all points drawn in canvas.
*/
static void clear_surface(void)
{
    cairo_t *cr = cairo_create(surface);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    cairo_destroy(cr);
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
               double     x,
               double     y)
{
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
static void draw_brush(
                       cairo_t   *cr,
                       double     x, 
                       double     y)
{
    g_print("%lf %lf\n", x, y);
    cairo_rectangle(cr, x - 3, y - 3, 6, 6);
    cairo_fill(cr);
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
    points_add(x, y);
    g_print("%d\n", points.count);
    cairo_t *cr;

    cr = cairo_create(surface);
    draw_brush(cr, x, y);
    draw_text(area, cr, x, y);
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
    points_clear();
    g_print("%d\n", points.count);

    clear_surface();
    gtk_widget_queue_draw(area);
}

/**
 * @brief (CALL_BACK) Function called whenever user closes main window
*/
static void close_window(void)
{
    points_destroy();
    if ( surface ) cairo_surface_destroy(surface);
}

/**
 * @brief (CALL_BACK) Function called whenever an option in "Algorithms"drop-down is sellected.
 * 
 * @param dropdown Dropdown selected
 * @param entry 
*/
static void algorithm_execution(GtkDropDown *dropdown,
                                gpointer     user_data)
{
    int dropdown_selected = gtk_drop_down_get_selected(dropdown);

    switch (dropdown_selected)
    {
    case 1:
        // DDA();
        break;
    case 2:
        // Bresenham();
        break;
    default:
        break;
    }

    g_print("Algorithm %d!\n", dropdown_selected);
}

/**
 * @brief (CALL_BACK) Function called whenever an option in "Algorithms"drop-down is sellected.
 * 
 * @param dropdown Dropdown selected
 * @param entry 
*/
static void drawings_execution(GtkDropDown *dropdown,
                               gpointer     user_data)
{
    int dropdown_selected = gtk_drop_down_get_selected(dropdown);

    switch (dropdown_selected)
    {
    case 1:
        // Line();
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

    g_print("Drawing: %d!\n", dropdown_selected);
}

/**
 * @brief (CALL_BACK) Function called whenever an option in "Transformation"drop-down is sellected, also, the value filled in "entry" is used.
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
        // translation(content);
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

void user_monitor_info(int *width, int *height)
{
    Display *display = XOpenDisplay(NULL);

    assert( display );
    Window root = DefaultRootWindow(display);
    XRRScreenResources *res = XRRGetScreenResources(display, root);

    // Forcing the choosen monitor to be the first one that appears
    XRROutputInfo *output_info = XRRGetOutputInfo(display, res, res->outputs[1]);
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
                   *main_input;
    GtkGesture     *drag, 
                   *press;
    GtkEntryBuffer *entry_buffer;


    const char *dropdown_content_algorithms[4] = {"Line Algorithms\0", "DDA\0", "Bresenham\0"};
    const char *dropdown_content_drawings[5] = {"Draw\0", "Line\0", "Polygon\0", "Circumference\0"};
    const char *dropdown_content_transformations[8] = {"Geometric Transformations\0", "Translate\0", "Rotate\0", "Scale\0", "X Reflection\0", "Y Reflection\0", "XY Reflection\0"};


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

    entry_buffer = gtk_entry_buffer_new(NULL, -1);
    main_input = gtk_entry_new_with_buffer(entry_buffer);
    gtk_entry_set_placeholder_text(GTK_ENTRY(main_input), "Transformation values here...");
    gtk_box_append(GTK_BOX(option_box), main_input);


    g_signal_connect(dropdown_algorithms, "notify::selected", G_CALLBACK(algorithm_execution), entry_buffer);
    g_signal_connect(dropdown_drawings, "notify::selected", G_CALLBACK(drawings_execution), entry_buffer);
    g_signal_connect(dropdown_transformations, "notify::selected", G_CALLBACK(transformation_execution), entry_buffer);
    gtk_window_present(GTK_WINDOW(window));

}

int main(int    argc,
         char** argv)
{
    GtkApplication *app;
    int status;

    points_init();

    app = gtk_application_new("GC.Thiago", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);

    points_destroy();

    return status;
}