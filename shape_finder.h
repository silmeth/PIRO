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

#include "straightener.h"

#include <iostream>
#include <stdio.h>

#include <cmath>

using namespace std;
using namespace cv;

extern vector<Point2f> corners_old;

extern int refresh_corners;

/*
 * Wyświetla obraz bezpośredni z kamery.
 */
void camera_raw_display(int num);
/*
 * Wyświetla Hue z HSV
 */
void camera_HUE_display(int num);
/*
 *
 */
void camera_straighten_display(int num, char* window_name);
/*
 *
 */
void camera_contours_display(int num, Straightener & straight);
/*
 * Sortuje narożniki w kolejności: górny lewy, górny prawy, dolny prawy, dolny lewy
 */
void sortCorners(vector<Point2f>& corners, Point2f center);
/*
 * Prostuje obraz
 */
bool straighten(Mat &src, Mat &dst, unsigned int rows, unsigned int cols);

vector<Point2f> find_corners(Mat &src, unsigned int rows, unsigned int cols);

#endif /* SHAPE_FINDER_H_ */
