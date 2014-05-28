/*
 * tests.cpp
 *
 *  Created on: May 18, 2014
 *      Author: jacek
 */
#include "tests.h"

char *win_name;
Mat test_src;

/// 3 lub 5
int canny_kernel_size = 3;
/// Najlepiej 1 lub 2
int canny_ratio = 1;
/// Najlepiej 50
int canny_low_threshold = 1;

int hough_rho = 1;
int hough_theta = 1;
int hough_voters = 1;
int hough_min_line_len = 1;
int hough_max_line_gap = 1;
vector<Vec4i> hough_lines;

void init_tests(char* window_name, Mat &src) {
	/// Copy window name
	win_name = window_name;
	test_src = Scalar::all(0);
	src.copyTo(test_src);
}

void test_canny_window(int, void*) {
	/// Create destination
	Mat temp;
	/// Stores detected edges
	Mat detected_edges;
	/// Copy source do detected edges
	test_src.copyTo(detected_edges);
	/// Apply blur
	blur(detected_edges, detected_edges, Size(3,3));
	// Apply Canny to destination Matrix
	Canny(detected_edges, detected_edges, canny_low_threshold, canny_low_threshold*canny_ratio, canny_kernel_size);
	test_src.copyTo(temp, detected_edges);
	imshow(win_name, temp);
}

void test_canny() {
	createTrackbar("Min Threshold:", win_name, &canny_low_threshold, 300, test_canny_window);
	createTrackbar("Ratio: ", win_name, &canny_ratio, 5, test_canny_window);
	createTrackbar("Kernel size:", win_name, &canny_kernel_size, 7, test_canny_window);
}

/// HOUGH
void test_hough_window(int, void*) {
	/// Create destination
	Mat temp;
	test_src.copyTo(temp);
	/// Apply blur
	blur(temp, temp, Size(3,3));
	/// Apply Canny to destination Matrix
	Canny(temp, temp, 50, 50, 3);
	/// Apply Probabilistic Hough Lines
	HoughLinesP(temp, hough_lines, (double)hough_rho, CV_PI/(double)(180*hough_theta), hough_voters,
				(double)hough_min_line_len, (double)hough_max_line_gap);
	/// Print found lines on temp
	for( size_t i = 0; i < hough_lines.size(); i++ ) {
	    Vec4i l = hough_lines[i];
	    line( temp, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, CV_AA);
	}
	imshow(win_name, temp);
}

void test_hough() {
	createTrackbar("Rho [deg]: ", win_name, &hough_rho, 9, test_hough_window);
	createTrackbar("Theta [deg]: ", win_name, &hough_theta, 9, test_hough_window);
	createTrackbar("Threshold: ", win_name, &hough_voters, 200, test_hough_window);
	createTrackbar("Min line len: ", win_name, &hough_min_line_len, 200, test_hough_window);
	createTrackbar("Max line gap: ", win_name, &hough_max_line_gap, 50, test_hough_window);
}

/// CONTOURS
/// TODO dokończyćs
void test_contours_window() {
	Mat temp;
	/// Znalezione kontury
	vector<vector<Point> > contours;
	/// Hierarchia konturów
	vector<Vec4i> hierarchy;

	test_src.copyTo(temp);
	blur(temp, temp, Size(3,3));
	/// Apply Canny to destination Matrix
	Canny(temp, temp, 100, 100, 3);
	findContours( temp, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
	/// Narysuj znalezione kontury
	Mat drawing = Mat::zeros( temp.size(), CV_8UC3 );
	for( unsigned int i=0; i< contours.size(); i++ ) {
			Scalar color = Scalar( 255, 255, 255);
			drawContours( drawing, contours, i, color, 1, 8, hierarchy, 0, Point() );
	}
	imshow(win_name, drawing);
}

void test_contours() {
	test_contours_window();
}

/// HOUGH for video input with no trackbar
/// Create destination
void hough_video_notrackbar(){
	int c;
	IplImage* color_img;
	CvCapture* cv_cap = cvCaptureFromCAM(1);
	cvNamedWindow("Video",0); // create window
	for(;;) {
		color_img = cvQueryFrame(cv_cap); // get frame
		if(color_img != 0) {
			Mat cam_mat(color_img);
			cvtColor( cam_mat, cam_mat, CV_BGR2GRAY );
			blur(cam_mat, cam_mat, Size(7,7));
			Canny(cam_mat, cam_mat, 50, 50, 3);
			///  !!!!!
			int erosion_type = 2;
			int erosion_size = 1;
			Mat element = getStructuringElement( erosion_type,
			                                       Size( 2*erosion_size + 1, 2*erosion_size+1 ),
			                                       Point( erosion_size, erosion_size ) );
			dilate(cam_mat, cam_mat, element);
			imshow("Video", cam_mat);
		}
		c = cvWaitKey(10); // wait 10 ms or for key stroke
		if(c == 27)
			break; // if ESC, break and quit
	}
	/* clean up */
	cvReleaseCapture( &cv_cap );
	cvDestroyWindow("Video");
}
