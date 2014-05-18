/*
 * tests.cpp
 *
 *  Created on: May 18, 2014
 *      Author: jacek
 */
#include "tests.h"

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
	/// Apply blur
	blur(temp, temp, Size(3,3));
	/// Apply Canny to destination Matrix
	Canny(temp, temp, 100, 100, 3);
	/// Apply Probabilistic Hough Lines
	HoughLinesP(temp, hough_lines, (double)hough_rho, (double)CV_PI/180*hough_theta, hough_voters,
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


