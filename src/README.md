# Files
## `circumference.c`
Contains the implementation of `include/circumference.h`. A Circumference is a structure that contains two `point_tt`; center and border, and also an identifier to differentiate instances of circumferences. You should guide yourself through the comments in the code and through the explanation below: 

<ol>
    <li>circumference_create(): Receives two points, the first one represents the center of the circumference, the second one is the border. Returns a newly instantiated Circumference struct;</li>
    <li>circumference_destroy(): Receives a circumference as paramether, destroys both points (center and border) and frees circumference pointer;</li>
    <li>circumference_get_points(): Receives a circumference and returns its center and border; </li>
    <li>circumference_radius(): Receives a circumference and returns its radius (Euclidian Distance between Center and Border);</li>
    <li>circumference_id(): Receives a circumference and returns its id.</li>
</ol>


## `clip.c`
Contains the implementation of `include/clipping.h`. A Clip is a rectangular structure that contains an identifier to differentiate instances of clips, and also an `array_tt` that must contain EXACTLY four `point_tt`. You should guide yourself through the comments in the code and through the explanation below: 

<ol>
    <li>clip_create(): Receives a pointer to FOUR points, number of points and a flag to define which algorithm to use (DDA or Bresenham). If number of points is lesser than NUM_CLIP_POINTS, an execption in thrown. Returns a newly instantiated Clip struct;</li>
    <li>clip_get_points(): Receives a clip and returns its points; </li>
    <li>clip_destroy(): Receives a clip, destroys array of points and frees clip pointer;</li>
    <li>clip_get_maxmin(): Receives a clip structure and returns its max and min based in how it was created in q_main.c;</li>
    <li>clip_id(): Receives a clip and returns its id.</li>
</ol>

## `color.c`
Contains the implementation of `include/color.h`. A Color is a structure that contains values of red, green and blue (RGB). Used to visually differentiate normal points from `clip->points`. You should guide yourself through the comments in the code and through the explanation below: 

<ol>
    <li>color_create(): Receives red, green and blue values. Returns a newly instantiated Color struct;</li>
    <li>clip_get_colors(): Receives a color pointer and returns its RGB; </li>
    <li>clip_destroy(): Receives a clip, destroys array of points and frees clip pointer.</li>
</ol>

## `line.c`
Contains the implementation of `include/line.h`. A Line is a structure that contains a variable to define which drawing algorithm was used to draw it (DDA or Bresenham), an identifier to differentiate instances of lines, a variable to check if line was clipped (If is inside of Clip area), and also four `point_tt` that are, respectively, Line's initial point, Line's final point, Line's initial clipped point, Line's final clipped point. You should guide yourself through the comments in the code and through the explanation below: 

<ol>
    <li>line_create(): Receives two points, initial and final, and a flag to define which algorithm to use (DDA or Bresenham). Initally a line is instatiated as not_clipped. Returns a newly instantiated Line struct;</li>
    <li>line_destroy(): Receives a line, destroys points and frees Line pointer;</li>
    <li>line_add_clipped_points(): Receives a line structure, two points - which are the clipped_initial and clipped_final points - and a flag to determine if line crosses Clip area, as is implemented in q_main.c;</li>
    <li>line_get_points(): Receives a line and returns its original (not_clipped) points. </li>
    <li>line_get_clipped_points(): Receives a line and returns its clipped points; </li>
    <li>line_get_algh(): Receives a line and returns its drawing algorithm. 1 = DDA, 2 = Bresenham. </li>
    <li>line_was_clipped(): Receives a line and returns if it was clipped or not; </li>
    <li>line_id(): Receives a line and returns its id.</li>
</ol>

## `point.c`
Contains the implementation of `include/point.h`. A Point is a structure that contains an identifier to differentiate instances of points, a variable to check if point was taken by any object (Circumference, Line, Polygon or Clip), its x and y coordinates and a `color_tt`. You should guide yourself through the comments in the code and through the explanation below: 

<ol>
    <li>point_create(): Receives a X and Y coordinates. Returns a newly instantiated Point structure;</li>
    <li>pont_destroy(): Receives a point, destroys point color and frees Line pointer;</li>
    <li>point_x_coord(): Receives a point structure and returns its x coordinate;</li>
    <li>point_y_coord(): Receives a point structure and returns its y coordinate;</li>
    <li>point_set_coord(): Receives a point and XY coordinates and sets them as new coordinates to point; </li>
    <li>point_id(): Receives a point and returns its id.</li>
    <li>point_take(): Receives a point and change its take variable to True. It implies that point is used as a point in an object (Circumference, Line, Polygon or Clip). </li>
    <li>point_define_color(): Receives a point and red, green and blue and defines it as point's new color; </li>
    <li>point_color(): Receives a point and return its color. </li>
</ol>

## `polygon.c`
Contains the implementation of `include/polygon.h`. A Polygon is a structure that contains a variable to define which drawing algorithm was used to draw it (DDA or Bresenham), an identifier to differentiate instances of polygons, a variable to check if line was clipped (If is inside of Clip area), and also two `array_tt` of  `point_tt` (size equals to `MAX_POINTS` defined in `include/polygon.h`, which implies that a `polygon_tt` might be made of `MAX_POINTS` `point_tt`), that is, respectively, an `array_tt` of original points and an `array_tt` of clipped points. You should guide yourself through the comments in the code and through the explanation below: 

<ol>
    <li>polygon_create(): Receives a pointer of points, a variable that defines amount of points to be taken, and a flag to define which algorithm to use (DDA or Bresenham). Initally a polygon is instatiated as not_clipped. Returns a newly instantiated Polygon struct;</li>
    <li>polygon_add_clipped_points(): Receives a polygon structure, a pointer of clipped points, and a flag to determine if polygon crosses Clip area, as is implemented in q_main.c;</li>
    <li>polygon_get_points(): Receives a polygon and returns its original (not_clipped) points. </li>
    <li>polygon_get_clipped_points(): Receives a polygon and returns its clipped points; </li>
    <li>polygon_destroy(): Receives a polygon, destroys points and frees Polygon pointer;</li>
    <li>polygon_get_algh(): Receives a polygon and returns its drawing algorithm. 1 = DDA, 2 = Bresenham. </li>
    <li>polygon_was_clipped(): Receives a polygon and returns if it was clipped or not; </li>
    <li>polygon_id(): Receives a polygon and returns its id.</li>
</ol>

## `q_main.c`
It is the main application code. Contains a global struct called Widgets, which is a struct that contains all used Widgets in interface. We can divide its functions in four categories: i) Interface; ii) Objects representation; iii) Operations in Objects; iv) Utils. You should guide yourself through the comments in the code and through the explanation below: 

<li>main(): Creates points', lines', polygons', circumferences' and clips's array, they are global variables and contains all Objects that are created throughout the code. Also, creates a gtk's application and starts the code.</li>

### I) Interface
Contains a global struct called Widgets, which is a struct that contains all GTK4's used Widgets in User's interface.

[GTK4 Documentation](https://docs.gtk.org/gtk4/index.html)
<ol>
    <li>activate(): Receives an application and controls all widgets and gestures used in User's interface. Makes all signal connections (i.e., some Widgets produces specific "signals" that should be listened in order to use the Widgets);</li>
    <li>user_monitor_info(): Uses X11's lib to get User's main screen width and height in order to create a big drawing area; </li>
    <li>cropping_selection(): CallBack function that is associated to "Clipping"'s dropdown. Whenever a User selects an option in "Clipping"'s dropdown a signal is listened "notify::selected" and cropping_selection is called, then a clipping algorithm is executed (Cohen-Sutherland or Liang-Barsky); </li>
    <li>transformation_execution(): CallBack function that is associated to "Transformation"'s dropdown. Whenever a User selects an option in "Transformation"'s dropdown a signal is listened "notify::selected" and transformation_execution is called, then a transformation algorithm is executed (Translation, Rotation, Scale, Reflections); </li>
    <li>drawings_execution(): CallBack function that is associated to "Draw"'s dropdown. Whenever a User selects an option in "Draw"'s dropdown a signal is listened "notify::selected" and drawings_execution is called, then a new Object (Line, Polygon, Circumference) is created; </li>
    <li>algorithms_execution(): CallBack function that is associated to "Algorithms"'s dropdown. Whenever a User selects an option in "Algorithms"'s dropdown a signal is listened "notify::selected" and algorithms_execution is called, then a line drawing algorithm is selected (DDA or Bresenham); </li>
    <li>close_window(): CallBack function called whenever application is closed, destroies all arrays of Objects (Points, Lines, Polygons, Circumferences and Clips);</li>
    <li>draw(): CallBack function called whenever User left-clicks in Drawing Area. If current number of points is not greater than MAX_POINTS (defined at polygon.h), creates a new Point at clicked position, draws it in Drawing Area and also write its coordinates (XY); </li>
    <li>draw_brush(): Function that, using cairo, draws a rectangle of size 6-6 at given XY; </li>
    <li>draw_text(): Function that, using cairo, writes a point's XY coordinates under it; </li>
    <li>draw_cb(): CallBack function that is called whenever drawing area is initialized, instantiating the surface where Users will paint; </li>
    <li>resize_cb(): CallBack function that is called whenever application window is resized. It removes all points in Drawing Area, if any, to prevent errors; </li>
    <li>clear_surface(): CallBack function that is called whenever User right-clicks in Drawing area. It removes all objects (Points, Lines, Polygons, Circumferences and Clips) in Drawing Area, if any. </li>
</ol>

### II) Object Representation
Contains five global arrays, which contains all drawn Objects (Points, Lines, Polygons, Circumferences, Clips), respectively.
<ol>
    <li>clip_structure(): Iterates through all points, select the not-taken in sequential pairs, since a Clip must have a rectangle shape, those will be the oposite side of our rectangle. Other two points are created inside the function and their XY are based on the two first points in order to create a perfect rectangle. The drawing algorithm for Clips is, always, DDA (author's choice). Since it creates other two points, if it goes beyond MAX_POINTS, throws an error. If there is not atleast 2 not-taken points, throws error.</li>
    <li>redraw_objects(): Iterates through all Object's arrays (Points, Lines, Polygons, Circumferences, Clips), check if they should be drawn (e.g., Lines and Polygons might not be if not inside a Clip area) and redraws them all. Lines and Polygons are redrawn following their pre-stabilished algorithm (DDA or Bresenham). Circumferences are always drawn with a different Bresenham's algorithm. Clips are always drawn with DDA (author's choice). If a point is not taken, it should also be redrawn.</li>
    <li>write_execution_time(): Used to write the execution time (in ms) of an algorithm.</li>
    <li>Line(): Iterates through all points, select the not-taken in sequential pairs and creates a new Line object, if there is a odd number of points, it implies that a point will not be taken. Draws line based on selected algorithm (DDA or Bresenham). If no drawing algorithm is selected throws error. If there is not atleast 2 not-taken points drawn, throws error.</li>
    <li>Polygon(): Iterates through all points, select all the not-taken sequentially. After selection, draws lines between all pairs of selected points. Draws line based on selected algorithm (DDA or Bresenham). If no drawing algorithm is selected throws error. If there is not atleast 3 not-taken points drawn, throws error.</li>
    <li>Circumference(): Iterates through all points, select the not-taken in sequential pairs and creates a new Circumference object, if there is a odd number of points, it implies that a point will not be taken. Draws Circumference based on a different Bresenham algorithm. If no drawing algorithm is selected throws error. If there is not atleast 2 not-taken points drawn, throws error.</li>
    <li>calculate_circumference_points(): Using Circumference's proportion, calculates the 2nd Octant of a Circumference and them replicate them all to draw the full Circumference;</li>
    <li>plot_circle_points(): Plots circumference's points based in its octants.</li>
    <li>number_taken_points(): Iterates through all points and returns the number of those that are taken by some object (Line, Polygon, Circumference, Clip);</li>
</ol>

### III) Operations in Objects
#### Clipping
<ol>
    <li>cohen_init(): Initializes a Clip structure and iterates through Line's and Polygon's array to check if their points are inside the Clip area, if positive, they will be the new clipped points of Object. If Line is not inside a Clip, don't draw it. If a Polygon's line is not inside a Clip, don't draw it (author's decision). Everytime that you perform this, new points will be created to each object inside the clipped area, so pay attention to MAX_POINS. If can't create a Clip structure, throws error. If there is no Clips created, throws error.</li>
    <li>cohen_sutherland(): Calculates new line's points XY coordinates using Cohen-Sutherland's algorithm idea. If points are not inside Clip area, returns NULL, otherwise, returns newly created points; </li>
    <li>region_code(): Used by cohen_sutherland(). Calculates the "binary" code based in Point's XY and where they are related to Clip area;</li>
    <li>liagn_barsky_init(): Initializes a Clip structure and iterates through Line's and Polygon's array to check if their points are inside the Clip area, if positive, they will be the new clipped points of Object. If Line is not inside a Clip, don't draw it. If a Polygon's line is not inside a Clip, don't draw it (author's decision). Everytime that you perform this, new points will be created to each object inside the clipped area, so pay attention to MAX_POINS. If can't create a Clip structure, throws error. If there is no Clips created, throws error.</li>
    <li>liang_barsky(): Calculates new line's points XY coordinates using Liang-Barsky's algorithm idea. If points are not inside Clip area, returns NULL, otherwise, returns newly created points;</li>
    <li>clip_test(): Checks if a DELTA(x) and DELTA(Y) of a point are related to Clip area.</li>
</ol>

#### Transformations
<ol>
    <li>translation(): Pattern: '(X,Y)' - Where X and Y are, both, integer values (positive or negative). Iterates through all Object's arrays and apply the specified transformation in Pattern in all points (Except for Clip's points). After Translating, redraw_objects is called;</li>
    <li>scale(): Pattern: '(X,Y)' - Where X and Y are, both, integer values (positive or negative). Positive values scale up object, negative values scale down object. Iterates through all Object's arrays and apply the specified scale in Pattern in all points (Except for Clip's points). After Scaling, redraw_objects is called;</li>
    <li>rotation(): Pattern: 'Xd' - Where X is integer value (positive or negative) ('d' stands for Degrees). Iterates through all Object's arrays, pins the first point of Object and apply the specified rotation in Pattern in all other points (Except for Clip's points). It implies that Object will rotate around pinned point. After rotation, redraw_objects is called;</li>
    <li>xreflection(): Iterates through all Object's arrays and invert the X value of all Object's points (Except for Clip's points). After XReflecting, redraw_objects is called;</li>
    <li>yreflection(): Iterates through all Object's arrays and invert the Y value of all Object's points (Except for Clip's points). After YReflecting, redraw_objects is called;</li>
    <li>xyreflection(): Iterates through all Object's arrays and invert the X and Y value of all Object's points (Except for Clip's points). After XYReflecting, redraw_objects is called.</li>
</ol>

### IV) Utils
<ol>
    <li>check_content(): Checks if informed Pattern in INPUT is correct to selected transformation. If positive, returns the values written in input, otherwise, returns NULL;</li>
    <li>read_from_until(): Reads an array of char up until a specified stop char and checks if, in given array of char, there is only numbers, '.', ',' and signals ('-' and '+').</li>
</ol>