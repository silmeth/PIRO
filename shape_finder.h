/*
 * shape_finder.h
 *
 *  Created on: May 18, 2014
 *      Author: jacek
 */

#ifndef SHAPE_FINDER_H_
#define SHAPE_FINDER_H_

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/contrib/contrib.hpp"


#include <iostream>
#include <stdio.h>

#include <cmath>

using namespace std;
using namespace cv;

/*
 * parameterized line
 */
typedef struct {
	double atana; // arc tangent a
	double b; // parameter b
	double len;
} par_line;


/*
 * Wyświetla obraz z kamery.
 */
void camera_raw_display(int num);
/*
 * Sortuje narożniki w kolejności: górny lewy, górny prawy, dolny prawy, dolny lewy
 */
void sortCorners(vector<Point2f>& corners, Point2f center);
/*
 * Prostuje obraz
 */
void straighten(Mat &src, Mat &dst, int rows, int cols);

#endif /* SHAPE_FINDER_H_ */
