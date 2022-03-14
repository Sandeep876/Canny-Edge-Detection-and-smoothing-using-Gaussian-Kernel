# Canny-Edge-Detection-and-smoothing-using-Gaussian-Kernel

Implemented Connected Components to identify contiguous edge segments 
and to color each edge component a random color
Subsequently, implemented a Gaussian Kernel to smooth the (x,y) positions of 
the surrounding points.
Firstly, done the edge detection using the canny method.
1. Canny edge detection 
2. Connected components to color each edge region using a different color3. 
Gaussian kernel to smooth the pixel locations of the edge pixels

Define (x^i , y^i) as smoothed edge positions
Define C1...CN as connected components of edge pixels

) Your program must compile and run with NO command arguments, NO user input 
It must read, and process the following images in a loop
 input: puppy.jpg output: out/puppy_<<file suffix>>
 input: pentagon.png output: out/pentagon_<<file suffix>>
 input: houses.jpg output: out/houses_<<file suffix>>
 input: tiger.jpg output: out/tiger_<<file suffix>>
 
 It must use relative paths, and the relative path must be the same 
directory as the source code.
 For Windows you must use forward slashes “out/” not “out\\” 
to specify your output path
 <<file suffix>> must produce each of the following
1_img.png
2_gray.png
3_blur.png
4_sobel.png
5_nonmax.png
6_edges.png
7_color_edges.png
8_smooth_edges.png
HINT: you are allowed to change the hyperparameters parameters for each 
image as such, you are encouraged to write a function and pass the 
hyperparameters as an argument.
r4) Your program must not use any external libraries, you are however allowed to 
use / modify the sample code as necessary
 NOTE: you are allowed to use internal libraries such as math.h if they are part of 
the ansi C standard library r5) Your program must run in less than 30 seconds on an 
I5 processor
