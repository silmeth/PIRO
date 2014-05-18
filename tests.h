/*
 * tests.h
 *
 *  Created on: May 18, 2014
 *      Author: jacek
 */

#ifndef TESTS_H_
#define TESTS_H_

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/contrib/contrib.hpp"

#include <iostream>
#include <stdio.h>
#include <cmath>

using namespace std;
using namespace cv;

Mat test_src;
char * win_name;

/// CANNY
int canny_low_threshold = 1;
int canny_ratio = 1;
int canny_kernel_size = 3;
/// HOUGH
int hough_rho = 1;
int hough_theta = 1;
int hough_voters = 1;
int hough_min_line_len = 1;
int hough_max_line_gap = 1;
vector<Vec4i> hough_lines;

/*
 * Inicjalizacja testów
 */
void init_tests(char* window_name, Mat &src);
/// CANNY
void test_canny();
void test_canny_window(int, void*);
/// HOUGH
void test_hough();
void test_hough_window(int, void*);
/// CONTOURS
void test_contours();
void test_contours_window();


#endif /* TESTS_H_ */
