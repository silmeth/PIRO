#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/contrib/contrib.hpp"

#include "shape_finder.h"
//#include "tests.h"

#include <iostream>
#include <stdio.h>

#include <cmath>

using namespace std;
using namespace cv;

Mat src, src_gray;
Mat dst;

char window_name[] = "Edge Map";


//vector<vector<Point> > contours; //contours of the paper sheet
//vector<Vec4i> hierarchy;
//
//vector<vector<Point> > approx_contours;
//
//void contours_window(int, void*) {
//	Mat temp;
//	canny_blur_filter(src_gray, temp, 3, 50, 3);
//	findContours( temp, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
//	/// Draw contours
//	Mat drawing = Mat::zeros( temp.size(), CV_8UC3 );
//	// Z tutka https://github.com/Itseez/opencv/blob/master/samples/cpp/contours2.cpp
//	approx_contours.resize(contours.size());
//	for(unsigned int i = 0; i < contours.size(); i++){
//		approxPolyDP(Mat(contours[i]), approx_contours[i], 5, true);
//	}
//	// Plotujemy !!!!
//	for( unsigned int i=0; i< approx_contours.size(); i++ ) {
//		Scalar color = Scalar( 255, 0, 0);
//		drawContours( drawing, contours, i, color, 1, 8, hierarchy, 0, Point() );
//		color = Scalar( 0, 255, 0);
//		drawContours( drawing, approx_contours, i, color, 1, 8, hierarchy, 0, Point() );
//	}
//	imshow(window_name, drawing);
//}


int main(int argc, const char** argv) {
	/// Load an image
	src = imread( "./img/benchmark.png" );
	if( !src.data )
	{ return -1; }
	/// Convert the image to grayscale
	cvtColor( src, src_gray, CV_BGR2GRAY );
	/// Create a window
	namedWindow( window_name, CV_WINDOW_AUTOSIZE );
	straighten(src_gray, dst, 300, 450);
	imshow(window_name, dst);
	waitKey(0);
}
