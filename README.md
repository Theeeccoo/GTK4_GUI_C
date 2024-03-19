# GTK4_GUI_C
A Guided User Interface (GUI) implemented in C to show, visually, the execution of few algorithms:
<ol>
    <li> Digital Differential Analyzer (DDA) - Lines and Polygons; </li> 
    <li> Bresenham - Lines, Polygons and Circumference; </li> 
    <li> Transformations - Translation, Scaling, Rotation, Reflections; </li>
    <li> Cohen-Sutherland - Clipping; </li> 
    <li> Liang-Barsky - Clipping. </li>
</ol>

Interface created using GTK4 (to create widgets) and cairo (to plot points). When executing code, User will be faced with a canvas and some dropdowns. User can draw points in Canvas and draw Lines or Polygons based on selected algorithms (DDA or Bresenham) and draw Circumferences based on Bresenham algorithm.

This project aims to complement the knowledge acquired in the Computer Graphics (CG) course - Pontifical University Catolic of Minas Gerais (PUC-MG), Computer Science 2024/1 - through the development of a practical application for the studied algorithms.

## Instalation & Execution
<ol>
    <li> Clone the repository: https://github.com/Theeeccoo/GTK4_GUI_C.git </li>
    <li> Navigate to the project directory: `cd GTK4_GUI_C/` </li>
    <li> Need to install some libraries before executing: </li>

```
chmod +x build.sh # << Enabling build.sh to be executable
sudo ./build.sh    
```
> ⚠️ Some libraries might not work in environments different than Ubuntu.
<li> After installation: </li>

```
make all # << Linking headers and libraries, and compiling all structures 
cd bin/
./main # Runs executable    
```
> If needed, run 'make clean' to delete binary folder
</ol>

## Interface
When User executes the main code, he will come across a big portion of screen (91%) that contains the Drawing Area, and the other portion (9%) being composed of dropdowns, MAIN INPUT and DEBUG TEXT.

In Drawing Area, User can left-click to draw points and left-click to clear Drawing Area. To execute algorithms, User must select values in dropdowns that are present in the inferior portion of the application.
<ol>
    <li>First dropdown is related to which drawing algorithm will be used. User can select either DDA or Bresenham;</li>
    <li>Second dropdown is related to which Object User wants to draw. If no drawing algorithm is previously selected, an WARNING will be shown in DEBUG TEXT;</li>
    <li>Second dropdown is related to which Transformation is going to be operated in Object's points. Make sure to check documentation to properly use Transformations;</li>
    <li>Third dropdown is related to which clipping algorithm is going to be executed. Make sure to check documentation to properly use it;</li>
    <li>MAIN INPUT is where you place the TEMPLATES for Transformations. If incorrect input is typed, DEBUG TEXT will warn you;</li>
    <li>DEBUG TEXT is where all possible User's errors will be warned when they occur. It also shows how long a function spent in its execution (in ms). Make sure to always check it to make sure that you are correctly operating the Interface.</li>
</ol>

## Algorithms' Theory
( For structures and implementations, check README.md present in `include/` and `src/` )
### [Digital Differential Analyzer (DDA)](https://en.wikipedia.org/wiki/Digital_differential_analyzer_(graphics_algorithm))
The DDA algorithm is a simple method used for drawing lines on a digital display. It operates by calculating the incremental values of x and y at each step and then rounding them to the nearest integer coordinates to plot the line.

The basic idea behind the DDA algorithm is to determine the slope of the line and then incrementally step along the longer axis (either x or y) while incrementing the other axis in proportion to the slope. By incrementing one axis by 1 unit and then calculating the corresponding value for the other axis based on the slope, the algorithm generates a series of points that approximate the line.
### [Bresenham](https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm)
Bresenham's line drawing algorithm is an efficient method used for drawing lines on a digital display by determining the points of the line that best approximate its path. It operates by taking advantage of integer arithmetic and the decision-making process to choose the next pixel along the line.

The key idea behind Bresenham's algorithm is to use the nextPoint distance to originalPoint. Instead of calculating the slope of the line and performing floating-point arithmetic, Bresenham's algorithm works with integer values and "error terms" to decide which pixel to turn on for each step along the line.
### [Cohen-Sutherland](https://en.wikipedia.org/wiki/Cohen%E2%80%93Sutherland_algorithm)
The Cohen-Sutherland algorithm is a line clipping algorithm used to clip a line segment against a rectangular clipping window. It classifies each endpoint of the line segment as being inside, outside, or crossing the clipping window, based on its position relative to the window's boundaries. Then, it clips the line segment against the window by determining intersections with the window's boundaries.

### [Liang-Barsky](https://en.wikipedia.org/wiki/Liang%E2%80%93Barsky_algorithm)
The Liang-Barsky algorithm is a line clipping algorithm used to clip a line segment against an arbitrary clipping window defined by four boundary lines. It is an improvement over the Cohen-Sutherland algorithm and is more efficient, especially for non-rectangular clipping regions.

The Liang-Barsky algorithm uses parametric line equations to determine the intersections of the line segment with the clipping window's boundaries. It computes values of t along the line segment, where 0 ≤ t ≤ 1, to represent points along the line segment.

## Creator
[Thiago de Campos Ribeiro Nolasco](https://github.com/Theeeccoo)

Contact: thiagodecampos1@gmail.com 
