# FILES
## `circumference.h`
Header that contains all information of "Circumference" structure. To check how struct and functions are implemented, check README.md at `src/`. It contains two different pointers: 
<ol>
    <li>circumference_tt: Pointer to a Circumference structure;</li>
    <li>const_circumference_tt: Pointer to a constant Circumference structure.</li>
</ol>
Also, we have different function's definitions:
<ol>
    <li>circumference_create;</li>
    <li>circumference_destroy;</li>
    <li>circumference_id;</li>
    <li>circumference_radius;</li>
    <li>circumference_get_points.</li>
</ol>

## `clipping.h`
Header that contains all information of "Clip" structure. To check how struct and functions are implemented, check README.md at `src/`. It contains two different pointers: 
<ol>
    <li>clip_tt: Pointer to a Clip structure;</li>
    <li>const_clip_tt: Pointer to a constant Clip structure.</li>
</ol>
Also, we have different function's definitions:
<ol>
    <li>clip_create;</li>
    <li>clip_destroy;</li>
    <li>clip_id;</li>
    <li>circumference_get_points;</li>
    <li>clip_get_maxmin.</li>
</ol>

Has a definition of MAX_CLIP_POINTS of 4, which prevents a Clip to have more than that many points in its structure.

## `color.h`
Header that contains all information of "Color" structure. To check how struct and functions are implemented, check README.md at `src/`. It contains two different pointers: 
<ol>
    <li>color_tt: Pointer to a Color structure;</li>
    <li>const_color_tt: Pointer to a constant Color structure.</li>
</ol>
Also, we have different function's definitions:
<ol>
    <li>color_create;</li>
    <li>color_destroy;</li>
    <li>color_get_points;</li>
</ol>

## `line.h`
Header that contains all information of "Line" structure. To check how struct and functions are implemented, check README.md at `src/`. It contains two different pointers: 
<ol>
    <li>line_tt: Pointer to a Line structure;</li>
    <li>const_line_tt: Pointer to a constant Line structure.</li>
</ol>
Also, we have different function's definitions:
<ol>
    <li>line_create;</li>
    <li>line_destroy;</li>
    <li>line_id;</li>
    <li>line_add_clipped_points;</li>
    <li>line_get_algh;</li>
    <li>line_was_clipped;</li>
    <li>line_get_points;</li>
    <li>line_get_points;</li>
    <li>line_get_clipped_points.</li>
</ol>

## `point.h`
Header that contains all information of "Point" structure. To check how struct and functions are implemented, check README.md at `src/`. It contains two different pointers: 
<ol>
    <li>point_tt: Pointer to a Point structure;</li>
    <li>const_point_tt: Pointer to a constant Point structure.</li>
</ol>
Also, we have different function's definitions:
<ol>
    <li>point_create;</li>
    <li>point_destroy;</li>
    <li>point_x_coord;</li>
    <li>point_y_coord;</li>
    <li>point_id;</li>
    <li>point_take;</li>
    <li>point_is_taken;</li>
    <li>point_define_color;</li>
    <li>point_color.</li>
</ol>

## `polygon.h`
Header that contains all information of "Polygon" structure. To check how struct and functions are implemented, check README.md at `src/`. It contains two different pointers: 
<ol>
    <li>polygon_tt: Pointer to a Polygon structure;</li>
    <li>const_polygon_tt: Pointer to a constant Polygon structure.</li>
</ol>
Also, we have different function's definitions:
<ol>
    <li>polygon_create;</li>
    <li>polygon_destroy;</li>
    <li>polygon_id;</li>
    <li>polygon_add_clipped_points;</li>
    <li>polygon_get_algh;</li>
    <li>polygon_was_clipped;</li>
    <li>polygon_get_points;</li>
    <li>polygon_get_clipped_points;</li>
</ol>