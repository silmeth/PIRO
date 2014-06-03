#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/contrib/contrib.hpp"

#include "tests.h"
#include "shape_finder.h"

using namespace std;

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

	Straightener straight;

	/// Load an image
	src = imread( "./img/benchmark.png" );
	if( !src.data )
	{ return -1; }
//	/// Convert the image to grayscale
//	cvtColor( src, src_gray, CV_BGR2GRAY );
//	/// Create a window
//	straighten(src_gray, dst, 450, 300);
//	init_tests(window_name, dst);
//	namedWindow( window_name, CV_WINDOW_AUTOSIZE );
//	test_hough();
	///camera_raw_display(1);
//	camera_straighten_display(1, window_name);
	camera_contours_display(1, straight);
//	hough_video_notrackbar();
//	camera_HUE_display(1);
	waitKey(0);
	return 0;
}

// TODO rysowanie linii po złączeniu houghem
// TODO progowanie z histogramu
