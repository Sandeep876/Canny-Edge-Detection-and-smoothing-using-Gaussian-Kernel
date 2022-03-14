#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "dc_image.h"
#include <time.h>

#define MIN(a,b)  ( (a) < (b) ? (a) : (b) )
#define MAX(a,b)  ( (a) > (b) ? (a) : (b) )
#define ABS(x)    ( (x) <= 0 ? 0-(x) : (x) )



double Gaussian_Distribution(int n,int sigma)
{

 return 1/((sqrt(2*3.14)*sigma))*exp((1/(2.f))*-1*pow((n/sigma),2));

}

double Distance(int x, int y, int xi, int yi, int sigma)
{

   //int sigma = 10;

   return Gaussian_Distribution((sqrt( (x-xi)*(x-xi) + (y-yi)*(y-yi))), sigma);
}


struct DisjointSet;

typedef struct DisjointSet 
{
	int r,g,b;
	int x,y;
	int rank;
	struct DisjointSet *parent;
}DisjointSet;

void DisjointSetMake_set(DisjointSet *curr)
{

	curr->parent=curr;
    curr->rank= 1;

}

void DisjointSetLink(DisjointSet *a,DisjointSet *b)
{

	if(a->rank > b->rank)
	{
		b->parent=a;
	}
	else{
		a->parent=b;
		if(a->rank==b->rank)
		{
			b->rank+=1;
		}
	}
}

DisjointSet *DisjointSetFindRoot(DisjointSet *curr)
{
	if(curr->parent!=curr)
	{
		curr->parent=DisjointSetFindRoot(curr->parent);
	}

	return curr->parent;
}

void DisjointSetUnion(DisjointSet *a, DisjointSet *b)
{
		DisjointSetLink(DisjointSetFindRoot(a),DisjointSetFindRoot(b));
}

DisjointSet **Disjointmalloc2d_array(int rows, int cols) {
	int z;
	DisjointSet **ptr = (DisjointSet**)malloc(rows*sizeof(DisjointSet*));
	for (z=0; y<rows; y++)
		ptr[z] = (DisjointSet*)calloc(cols,sizeof(DisjointSet));
	return ptr;
}

//--------------------------------------------------
//--------------------------------------------------
// The following "run" function runs the entire algorithm
//  for a single vision file
//--------------------------------------------------
//--------------------------------------------------

void run(const char *infile, const char *outpre, int canny_thresh, int canny_blur,int distance_threshold,int sigma)
{
	int y,x;
	int rows, cols, chan;
	double value,x_summation=0,y_summation=0,total_summation=0;
	int component_x=0,component_y=0;
	
	//int sigma = 10;
	//-----------------
	// Read the image    [y][x][c]   y number rows   x cols  c 3
	//-----------------
	byte ***img = LoadRgb(infile, &rows, &cols, &chan);
	printf("img %p rows %d cols %d chan %d\n", img, rows, cols, chan);

	char str[4096];
	sprintf(str, "out/%s_1_img.png", outpre);
	SaveRgbPng(img, str, rows, cols);

	//-----------------
	// Convert to Grayscale
	//-----------------
	byte **gray = malloc2d(rows, cols);
	for (y=0; y<rows; y++){
		for (x=0; x<cols; x++) {
			int r = img[y][x][0];   // red
			int g = img[y][x][1];   // green
			int b = img[y][x][2];   // blue
			gray[y][x] =  (r+g+b) / 3;
		}
	}

	sprintf(str, "out/%s_2_gray.png", outpre);
	SaveGrayPng(gray, str, rows, cols);

	//-----------------
	// Box Blur   ToDo: Gaussian Blur is better
	//-----------------

	// Box blur is separable, so separately blur x and y
	int k_x=canny_blur, k_y=canny_blur;

	// blur in the x dimension
	byte **blurx = (byte**)malloc2d(rows, cols);
	for (y=0; y<rows; y++) {
		for (x=0; x<cols; x++) {

			// Start and end to blur
			int minx = x-k_x/2;      // k_x/2 left of pixel
			int maxx = minx + k_x;   // k_x/2 right of pixel
			minx = MAX(minx, 0);     // keep in bounds
			maxx = MIN(maxx, cols);

			// average blur it
			int x2;
			int total = 0;
			int count = 0;
			for (x2=minx; x2<maxx; x2++) {
				total += gray[y][x2];    // use "gray" as input
				count++;
			}
			blurx[y][x] = total / count; // blurx is output
		}
	}

	sprintf(str, "out/%s_3_blur_just_x.png", outpre);
	SaveGrayPng(blurx, str, rows, cols);

	// blur in the y dimension
	byte **blur = (byte**)malloc2d(rows, cols);
	for (y=0; y<rows; y++) {
		for (x=0; x<cols; x++) {

			// Start and end to blur
			int miny = y-k_y/2;      // k_x/2 left of pixel
			int maxy = miny + k_y;   // k_x/2 right of pixel
			miny = MAX(miny, 0);     // keep in bounds
			maxy = MIN(maxy, rows);

			// average blur it
			int y2;
			int total = 0;
			int count = 0;
			for (y2=miny; y2<maxy; y2++) {
				total += blurx[y2][x];    // use blurx as input
				count++;
			}
			blur[y][x] = total / count;   // blur is output
		}
	}

	sprintf(str, "out/%s_3_blur.png", outpre);
	SaveGrayPng(blur, str, rows, cols);


	//-----------------
	// Take the "Sobel" (magnitude of derivative)
	//  (Actually we'll make up something similar)
	//-----------------

	byte **sobel = (byte**)malloc2d(rows, cols);

	for (y=0; y<rows; y++) {
		for (x=0; x<cols; x++) {
			int mag=0;

			if (y>0)      mag += ABS((int)blur[y-1][x] - (int)blur[y][x]);
			if (x>0)      mag += ABS((int)blur[y][x-1] - (int)blur[y][x]);
			if (y<rows-1) mag += ABS((int)blur[y+1][x] - (int)blur[y][x]);
			if (x<cols-1) mag += ABS((int)blur[y][x+1] - (int)blur[y][x]);

			int out = 3*mag;
			sobel[y][x] = MIN(out,255);
		}
	}


	sprintf(str, "out/%s_4_sobel.png", outpre);
	SaveGrayPng(sobel, str, rows, cols);

	//-----------------
	// Non-max suppression
	//-----------------
	byte **nonmax = malloc2d(rows, cols);    // note: *this* initializes to zero!

	for (y=1; y<rows-1; y++)
	{
		for (x=1; x<cols-1; x++)
		{
			// Is it a local maximum
			int is_y_max = (sobel[y][x] > sobel[y-1][x] && sobel[y][x]>=sobel[y+1][x]);
			int is_x_max = (sobel[y][x] > sobel[y][x-1] && sobel[y][x]>=sobel[y][x+1]);
			if (is_y_max || is_x_max)
				nonmax[y][x] = sobel[y][x];
			else
				nonmax[y][x] = 0;
		}
	}

	sprintf(str, "out/%s_5_nonmax.png", outpre);
	SaveGrayPng(nonmax, str, rows, cols);

	//-----------------
	// Final Threshold
	//-----------------
	byte **edges = malloc2d(rows, cols);    // note: *this* initializes to zero!

	for (y=0; y<rows; y++) {
		for (x=0; x<cols; x++) {
			if (nonmax[y][x] > canny_thresh)
				edges[y][x] = 255;
			else
				edges[y][x] = 0;
		}
	}

	sprintf(str, "out/%s_6_edges.png", outpre);
	SaveGrayPng(edges, str, rows, cols);


	

	struct DisjointSet **image_pixels = Disjointmalloc2d_array(rows,cols);

	for(int y=0; y<rows; y++)
	{
   		for(int x=0; x<cols; x++)
		   {
      		image_pixels[y][x].x = y;
      		image_pixels[y][x].y = x;

      		DisjointSetMake_set(&image_pixels[y][x]);

   		}
	}


	for(int y=0; y<rows; y++)
	{
   		for(int x=0; x<cols; x++)
		   {

      		if(edges[y][x]==255)
			  {

						int d = 1;

						while (d <= distance_threshold) {

         			if(x+d < cols && edges[y][x+d] == 255){

               			DisjointSetUnion(&image_pixels[y][x], &image_pixels[y][x+d]);

         			}

							if(y+d < rows && edges[y+d][x] == 255){

               			DisjointSetUnion(&image_pixels[y][x], &image_pixels[y+d][x]);

         			}

							d = d+1;
      			}

      		}
   		}


	}


	byte ***Connect_components = malloc3d_array(rows, cols, chan);
	// Making pictures colorful:
    y=0;
	while(y<rows)
	{
		x=0;
		while(x<columns){

			if(edges[y][x]==255){

				if(image_pixels[y][x].parent->r == 0 && image_pixels[y][x].parent->g == 0 && image_pixels[y][x].parent->b == 0){

					image_pixels[y][x].parent->r=rand()%255;
					image_pixels[y][x].parent->g=rand()%255;
					image_pixels[y][x].parent->b=rand()%255;
				}

					image_pixels[y][x].r = image_pixels[y][x].parent->r;
					image_pixels[y][x].g = image_pixels[y][x].parent->g;
					image_pixels[y][x].b = image_pixels[y][x].parent->b;

					Connect_components[y][x][0] = image_pixels[y][x].r;
					Connect_components[y][x][1] = image_pixels[y][x].g;
					Connect_components[y][x][2] = image_pixels[y][x].b;

			}else{
					Connect_components[y][x][0] = 0;
					Connect_components[y][x][1] = 0;
					Connect_components[y][x][2] = 0;
			}
		}
	}
	sprintf(str, "out/%s_7_color_edges.png", outpre);
	SaveRgbPng(Connect_components, str, rows, cols);

	// Smoothing Function

    y=0;
	while (y<rows)
	{
			x=0;
			while (x<cols)
			{

				x_summation=0.0;
				y_summation=0.0;
				total_summation=0.0;

        if(edges[y][x] == 0)
		{
					continue;
			  }

				int i_start_pixels = y-2*sigma;
				int i_end_pixels  = y+2*sigma;
				int j_start_pixels = x-2*sigma;
				int j_end_pixels  = x+2*sigma;

				if ((y-2*sigma) <= 0)
					i_start_pixels = 0;

				if ((y+2*sigma) >= rows)
					i_end_pixels = rows;

				if ((x-2*sigma) <= 0)
					j_start_pixels = 0;

				if ((x+2*sigma) >= cols)
					j_end_pixels = cols;

        int i=i_start_pixels;
		while(i<i_end_pixels)
		{
					int j=j_start_pixels;
					while(j<j_end_pixels)
					{

						if (edges[i][j] == 0)
						{
							continue;
						}

            if(image_pixels[y][x].parent->x == image_pixels[i][j].parent->x && image_pixels[y][x].parent->y == image_pixels[i][j].parent->y){

							value = Distance(image_pixels[i][j].x, image_pixels[i][j].y, image_pixels[y][x].x, image_pixels[y][x].y, sigma);
							x_summation = (x_summation + value*image_pixels[i][j].x);
							y_summation = (y_summation + value*image_pixels[i][j].y);
							total_summation = (total_summation + value);
            }

          }
        }

				component_x = (int) (x_summation/total_summation);
				component_y = (int) (y_summation/total_summation);

				Connect_components[component_x][component_y][0] = 255;
				Connect_components[component_x][component_y][1] = 255;
				Connect_components[component_x][component_y][2] = 255;

      }

   	}

	sprintf(str, "out/%s_8_smooth_edges.png", outpre);
	SaveRgbPng(Connect_components, str, rows, cols);

	// Weight function

	// Gaussian Distribution implementation


}


int main()
{
	system("mkdir out");


	//
	// Main simply calls the run function
	//  with different parameters for each image
	//
	run("puppy.jpg", "puppy", 45, 25, 10, 10);
	run("pentagon.png", "pentagon", 45, 10, 10, 5);
	run("tiger.jpg", "tiger", 45, 10, 9, 5);
	run("houses.jpg", "houses", 45, 10, 4, 5);


	printf("Done!\n");
}
