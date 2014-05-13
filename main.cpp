#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/contrib/contrib.hpp"


#include <iostream>
#include <stdio.h>

using namespace std;
using namespace cv;

Mat src, src_gray;
Mat dst, detected_edges;

int edgeThresh = 1;
int canny_low_threshold = 50;
int max_canny_low_threshold = 100;
int ratio = 3;
int kernel_size = 3;
char* window_name = "Edge Map";

int min_adapt_max_val = 0;
int min_adapt_block_size = 1;

int canny_kernel_size = 1;
int max_canny_kernel_size = 200;

std::vector<cv::Vec4i> hough_lines;
int hough_rho = 1;
int hough_theta = 1;
int hough_threshold = 20;
int hough_min_line_length = 30;
int hough_max_line_gap = 10;

vector<vector<Point> > contours;
vector<Vec4i> hierarchy;

vector<vector<Point> > approx_contours;

struct

void camera_raw_display(){
	int c;
	IplImage* color_img;
	CvCapture* cv_cap = cvCaptureFromCAM(1);
	cvNamedWindow("Video",0); // create window
	for(;;) {
		color_img = cvQueryFrame(cv_cap); // get frame
		if(color_img != 0)
			cvShowImage("Video", color_img); // show frame
		c = cvWaitKey(10); // wait 10 ms or for key stroke
		if(c == 27)
			break; // if ESC, break and quit
	}
	/* clean up */
	cvReleaseCapture( &cv_cap );
	cvDestroyWindow("Video");
}

void canny_blur_filter(Mat &src, Mat &dst, int kernel_size, int low_treshold, int ratio){
	// Create destination
	dst = Scalar::all(0);
	// Stores detected edges
	Mat detected_edges;
	// Copy source do detected edges
	src.copyTo(detected_edges);
	blur(detected_edges, detected_edges, Size(3,3));
	// Apply Canny to destination Matrix
	Canny(detected_edges, detected_edges, low_treshold, low_treshold*ratio, kernel_size);
	src.copyTo(dst, detected_edges);
}

void canny_window(int, void*){
	blur(src_gray, dst, Size(3,3));
	Canny(dst, dst, 100, 100, 3);
	imshow( window_name, dst );
}

void adaptive_treshold(Mat &src, Mat &dst, int max_value, int block_size, double threshold_constant){
	src.copyTo(dst);
	blur(dst, dst, Size(3,3));
	adaptiveThreshold(src, dst, (double)max_value, ADAPTIVE_THRESH_MEAN_C,
			THRESH_BINARY, block_size, threshold_constant);
}

void adaptive_threshold_window(int, void*){
	adaptive_treshold(src_gray, dst, min_adapt_max_val, min_adapt_block_size*2 + 1, 0.0);
	imshow(window_name, dst);
}

void hough_canny_window(int, void*){
	Mat temp;
	canny_blur_filter(src_gray, temp, 3, 50, 3);
	HoughLinesP(temp, hough_lines, hough_rho, CV_PI/1800*hough_theta, hough_threshold,
				(double)hough_min_line_length, (double)hough_max_line_gap);
	temp.copyTo(dst);
	for( size_t i = 0; i < hough_lines.size(); i++ )
	  {
	    Vec4i l = hough_lines[i];
	    line( dst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, CV_AA);
	  }
	imshow(window_name, dst);
}

void contours_window(int, void*){
	Mat temp;
	canny_blur_filter(src_gray, temp, 3, 50, 3);
	findContours( temp, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
	/// Draw contours
	Mat drawing = Mat::zeros( temp.size(), CV_8UC3 );
	// Z tutka https://github.com/Itseez/opencv/blob/master/samples/cpp/contours2.cpp
	approx_contours.resize(contours.size());
	for(int i=0; i<contours.size(); i++){
		approxPolyDP(Mat(contours[i]), approx_contours[i], 5, true);
	}
	// Plotujemy !!!!
	for( unsigned int i=0; i< approx_contours.size(); i++ ){
		Scalar color = Scalar( 255, 0, 0);
		drawContours( drawing, contours, i, color, 1, 8, hierarchy, 0, Point() );
		color = Scalar( 0, 255, 0);
		drawContours( drawing, approx_contours, i, color, 1, 8, hierarchy, 0, Point() );
	}
	imshow(window_name, drawing);
}

void straighten(Mat &src, Mat &dst){
	std::vector<cv::Vec4i> slines;
	Mat temp;
	blur(src, temp, Size(3,3));
	Canny(temp, temp, 100, 100, 3);
	HoughLinesP(temp, slines, 1, CV_PI/360, 65, 80, 10);
	Mat drawing = Mat::zeros( temp.size(), CV_8UC3 );
	drawing = Scalar::all(0);
	for( unsigned int i = 0; i < slines.size(); i++ )
	  {
		Vec4i l = slines[i];
		line( drawing, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, CV_AA);
	  }
	cout << slines.size();



	imshow( window_name, drawing );
}

int main(int argc, const char** argv){
	/// Load an image
	src = imread( "/home/jacek/Studia/PIRO/face_rec_test/benchmark.png" );
	if( !src.data )
	{ return -1; }
	/// Convert the image to grayscale
	cvtColor( src, src_gray, CV_BGR2GRAY );
	/// Create a window
	namedWindow( window_name, CV_WINDOW_AUTOSIZE );

	/*
	 * Trackbar Canny Threshold detector
	 */
	createTrackbar("Min Threshold:", window_name, &canny_low_threshold, max_canny_low_threshold, canny_window);
	//createTrackbar("Kernel size:", window_name, &canny_kernel_size, max_canny_kernel_size, canny_window);
	/*
	 * Trackbar Adaptive Threshold
	 */
	//createTrackbar("max_value:", window_name, &min_adapt_max_val, 255, adaptive_threshold_window);
	//createTrackbar("block_size:", window_name, &min_adapt_block_size, 500, adaptive_threshold_window);

	/*
	 * Trackbar Hough Transform
	 */
	/*
	// 1 by tutorial
	createTrackbar("rho:", window_name, &hough_rho, 5, hough_canny_window);
	// 10 by tutorial
	createTrackbar("theta:", window_name, &hough_theta, 20, hough_canny_window);
	// 70 by tutorial
	createTrackbar("threshold:", window_name, &hough_threshold, 200, hough_canny_window);
	// 30 by tutorial
	createTrackbar("min_line_length:", window_name, &hough_min_line_length, 200, hough_canny_window);
	// 10 by tutorial
	createTrackbar("max_line_gap:", window_name, &hough_max_line_gap, 200, hough_canny_window);
	createTrackbar("max_line_gap:", window_name, &hough_max_line_gap, 200, hough_canny_window);
	*/
	// contours_window(5, &min_adapt_block_size);
	straighten(src_gray, dst);
	waitKey(0);
}
