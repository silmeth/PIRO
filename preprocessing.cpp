/*
 * preprocessing.cpp
 *
 *  Created on: Jun 3, 2014
 *      Author: jacek
 */

#include <iostream>

#include "preprocessing.h"

using namespace std;
using namespace cv;

Preprocessing:: Preprocessing(void) : straight(640, 480) {
	/// Enter some points to last_corners
	Point p = Point(10, 10);
	vector <Point> temp_vec;
	for(int k=0; k<4; k++){
		temp_vec.push_back(p);
		avg_corners.push_back(p);
	}
	for (int i=0; i<3; i++){
		last_corners.push_back(temp_vec);
	}
}

/*
 * src - BGR Mat
 * returns list of page corners
 */
vector<Point> Preprocessing:: getCorners(const Mat & src){
	Mat frameHSV;
	cvtColor(src, frameHSV, CV_BGR2HSV);
	Mat Sat = Mat(frameHSV.rows, frameHSV.cols, CV_8UC1);
	int ch[] = { 1, 0 };
	// Get Saturation from HSV
	mixChannels( &frameHSV, 1, &Sat, 1, ch, 1 );
	// Blur
	blur(Sat, Sat, Size(3,3));
	// Apply adaptive thresholding
	adaptiveThreshold(Sat, Sat, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 71, 30);

	Mat temp3;
	Sat.copyTo(temp3);
//	imshow("VIDEO", temp3);

	// Dilation
	int erosion_type = 2;
	int erosion_size = 1;
	Mat element = getStructuringElement( erosion_type,
										   Size( 2*erosion_size + 1, 2*erosion_size+1 ),
										   Point( erosion_size, erosion_size ) );
	/// Tripple erosion
	erode(Sat, Sat, element);
	erode(Sat, Sat, element);
	erode(Sat, Sat, element);
	/// Dilation
	dilate(Sat, Sat, element);

	/// Find contours
	vector<vector<Point> > contours; //contours of the paper sheet
	vector<Point> approx_contour; //approx contours of the paper sheet
	vector<Vec4i> hierarchy;
	// Find contours in Sat
	findContours( Sat, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE, Point(0, 0) );
	Scalar color = Scalar( 255, 255, 255);
	Scalar color2 = Scalar(0, 255, 0);
	/// Index of the longest contour
	int ind_cnt1 = -1;
	/// Index of the second longest contour
	int ind_cnt2 = -1;
	/// Length of the longest contour
	int cnt1_len = -1;
	/// Length of second the longest contour
	int cnt2_len = -1;
	/// Find 2 longest contours
	for(unsigned int i = 0; i < contours.size(); i++){
		/// If current is the longest
		if (arcLength(contours[i], true) > cnt1_len && arcLength(contours[i], true) >cnt2_len){
			cnt2_len = cnt1_len;
			cnt1_len = arcLength(contours[i], true);
			ind_cnt2 = ind_cnt1;
			ind_cnt1 = i;
		}
		/// If current is the second longest
		else if(arcLength(contours[i], true) <= cnt1_len && arcLength(contours[i], true) >cnt2_len){
			cnt2_len = arcLength(contours[i], true);
			ind_cnt2 = i;
		}
	}
	/// If there are 2 countours in the picture
	if(ind_cnt2 >= 0){
		/// Approximate second longest contour (hopefully page)
		Mat countour_mat(contours[ind_cnt2]);
		approxPolyDP(countour_mat, approx_contour, 50, true);

		/// Contour is a rectangle  - sure it is
		if(approx_contour.size() == 4){
			bool is_border = false;
			for (int i = 0; i < 4; i++){
				/// If point is on the border
				if(! (approx_contour[i].x > 10 && approx_contour[i].x < Sat.cols-10 &&
						approx_contour[i].y > 10 && approx_contour[i].y < Sat.rows-10)){
					is_border = true;
				}
			}
			/// No point is on border of image
			if(is_border == false){
				last_corners[0] = last_corners[1];
				last_corners[1] = last_corners[2];
				last_corners[2] = approx_contour;
				/// Sort corners
				for(int i=0; i<3; i++){
					straight.setCorners(last_corners[i]);
					last_corners[i] = straight.getCorners();
				}
			}
		}
	}
	/// Draw contours and corners
	Mat drawing = Mat::zeros( Sat.size(), CV_8UC3 );
	vector<vector<Point> > temp;
	temp.push_back(approx_contour);
	drawContours( drawing, temp, 0, color2, 1, 8, NULL, 0, Point() );
	/// Average last 3 sets of corners
	avgCorners();
	/// Display avg set of corners
	return avg_corners;
}

/*
 * Improved version working on HSV V channel and otsu adaptive threshold on grayscale image
 */
vector<Point> Preprocessing:: getCorners2(const Mat & src){
	Scalar color2 = Scalar( 0, 0, 255);
	Mat gray, hsv;
	cvtColor(src, gray, CV_BGR2GRAY);
	cvtColor(src, hsv, CV_BGR2HSV);
	Mat v = Mat(hsv.rows, hsv.cols, CV_8UC1);
	int ch[] = { 1, 0 };
	// Get Saturation from HSV
	mixChannels( &hsv, 1, &v, 1, ch, 1 );
	// Blur
	blur(gray, gray, Size(5,5));
	blur(v, v, Size(5,5));
	double otsu_gray, otsu_v;
	otsu_gray = threshold(gray, gray, 0, 255, THRESH_BINARY + THRESH_OTSU);
	otsu_v = threshold(v, v, 0, 255, THRESH_BINARY + THRESH_OTSU);
	Mat gray_binary, v_binary;
	threshold(gray, gray_binary, otsu_gray, 255, THRESH_BINARY);
	v_binary = threshold(v, v_binary, otsu_v, 255, THRESH_BINARY);
	Mat merged = min(v_binary, gray_binary);
	// Dilation and erosion
	int erosion_type = 2;
	int erosion_size = 2;
	Mat element = getStructuringElement( erosion_type,
										   Size( 2*erosion_size + 1, 2*erosion_size+1 ),
										   Point( erosion_size, erosion_size ) );
	for(int i = 0; i < 3; i++){
		erode(merged, merged, element);
	}
	for(int i = 0; i < 3; i++){
		dilate(merged, merged, element);
	}
	Canny(merged, merged, 10, 20);
	vector<vector<Point> > contours;
	vector<Point> approx_contour;
	vector<Vec4i> hierarchy;
	findContours( merged, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE, Point(0, 0) );

	/// Index of the longest contour
		int ind_cnt1 = -1;
		/// Index of the second longest contour
		int ind_cnt2 = -1;
		/// Length of the longest contour
		int cnt1_len = -1;
		/// Length of second the longest contour
		int cnt2_len = -1;
		/// Find 2 longest contours
		for(unsigned int i = 0; i < contours.size(); i++){
			/// If current is the longest
			if (arcLength(contours[i], true) > cnt1_len && arcLength(contours[i], true) >cnt2_len){
				cnt2_len = cnt1_len;
				cnt1_len = arcLength(contours[i], true);
				ind_cnt2 = ind_cnt1;
				ind_cnt1 = i;
			}
			/// If current is the second longest
			else if(arcLength(contours[i], true) <= cnt1_len && arcLength(contours[i], true) >cnt2_len){
				cnt2_len = arcLength(contours[i], true);
				ind_cnt2 = i;
			}
		}
		/// If there are 2 countours in the picture
		if(ind_cnt2 >= 0){
			/// Approximate second longest contour (hopefully page)
			Mat countour_mat(contours[ind_cnt2]);
			approxPolyDP(countour_mat, approx_contour, 50, true);

			/// Contour is a rectangle  - sure it is
			if(approx_contour.size() == 4){
				bool is_border = false;
				for (int i = 0; i < 4; i++){
					/// If point is on the border
					if(! (approx_contour[i].x > 10 && approx_contour[i].x < src.cols-10 &&
							approx_contour[i].y > 10 && approx_contour[i].y < src.rows-10)){
						is_border = true;
					}
				}
				/// No point is on border of image
				if(is_border == false){
					/// Check if are is neither too big nor to small (15-80%)
					if(contourArea(approx_contour) > 0.15*src.cols*src.rows && contourArea(approx_contour) < 0.80*src.cols*src.rows){
							last_corners[0] = last_corners[1];
							last_corners[1] = last_corners[2];
							last_corners[2] = approx_contour;
							/// Sort corners
							for(int i=0; i<3; i++){
								straight.setCorners(last_corners[i]);
								last_corners[i] = straight.getCorners();
							}
					}
				}
			}
		}
		/// Draw contours and corners
		/// Average last 3 sets of corners
		avgCorners();
		/// Display avg set of corners
		//return last_corners[2];
		return avg_corners;
}

/*
 * Merges two matrixes into one for easier display.
 * Supports only matrixes of the same size!
 */
Mat Preprocessing:: mergeMatrixes(const Mat & mat_l, const Mat & mat_r){
	if(mat_l.rows != mat_r.rows || mat_l.cols != mat_r.cols){
		Mat empty;
		empty.release();
		return empty;
	}
	else{
		// Convert to 8 bit, 3 channel
		Mat left = mat_l.clone();
		Mat right = mat_r.clone();
		left.convertTo(left, CV_8UC3);
		right.convertTo(right, CV_8UC3);
		Mat merged = Mat(left.rows, left.cols*2, CV_8UC3);
		left.copyTo(merged(Rect(0, 0, left.cols, left.rows)));
		right.copyTo(merged(Rect(left.cols, 0, right.cols, right.rows)));
		return merged;
	}
}

// Average last 3 sets of corners
void Preprocessing:: avgCorners(){
	vector<Point> temp_vec;
	Point zero_point = Point(0, 0);
	for(int i=0; i<4; i++){
		temp_vec.push_back(zero_point);
	}
	for(int i=0; i<3; i++){
		for(int k=0; k<4; k++){
			temp_vec[k] += last_corners[i][k];
		}
	}
	for(int k=0; k<4; k++){
		avg_corners[k].x = temp_vec[k].x / 3;
		avg_corners[k].y = temp_vec[k].y / 3;
	}
}

// Return empty vector of vectors if no shapes found
vector<vector<Point> > Preprocessing:: getShapes(const Mat & src){
	vector<Point> approx_cnt;
	vector<Vec4i> triangle_hier;
	vector<Vec4i> source_hier;
 	vector<vector<Point> > triangle_cnt;
	vector<vector<Point> > source_cnt;
	Mat triangle = imread("./img/triangle.png", CV_LOAD_IMAGE_GRAYSCALE);
	Mat source;
	cvtColor(src, source, CV_BGR2GRAY);
	blur(source, source, Size(5,5));
	double source_th, triangle_th;
	Mat tempMat = src.clone();
	source_th = threshold(source, tempMat, 0, 255, THRESH_BINARY + THRESH_OTSU);
	triangle_th = threshold(triangle, tempMat, 0, 255, THRESH_BINARY + THRESH_OTSU);
	threshold(source, source, source_th, 255, THRESH_BINARY);
	threshold(triangle, triangle, triangle_th, 255, THRESH_BINARY);
	findContours( source, source_cnt, source_hier, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE, Point(0, 0) );
	findContours( triangle, triangle_cnt, triangle_hier, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE, Point(0, 0) );
	//HoughCircles(source, circles, CV_HOUGH_GRADIENT, 2, 40, 100, 200, 30, 200);
	if(triangle_cnt.size() > 0){
		cout << "tr_cnt: " << triangle_cnt.size() << " src_cnt: " << source_cnt.size() << endl;
		for(unsigned int i = 0; i < source_cnt.size(); i++){
			if(contourArea(source_cnt[i]) < 0.2*src.cols*src.rows){
				if(matchShapes(source_cnt[i], triangle_cnt[0], 1, 0.0) > 0.25){
					triangles.push_back(source_cnt[i]);
				}
				else{
					approx_cnt.clear();
					approxPolyDP(source_cnt[i], approx_cnt, 5, true);
					if(approx_cnt.size() == 4){
						rectangles.push_back(source_cnt[i]);
					}
//					else if(approx_cnt.size() == 8){
//						other_shapes.push_back(source_cnt[i]);
//					}
				}
			}
		}
	}
	return triangles;
}

vector<vector<Point> > Preprocessing:: getTriangles(){
	vector<vector<Point> > temp = triangles;
	triangles.clear();
	return temp;
}

vector<vector<Point> > Preprocessing:: getRectangles(){
	vector<vector<Point> > temp = rectangles;
	rectangles.clear();
	return temp;
}

vector<Vec3f> Preprocessing:: getCircles(){
	vector<Vec3f> temp = circles;
	circles.clear();
	return temp;
}

vector<vector<Point> > Preprocessing:: getOtherShapes(){
	vector<vector<Point> > temp = other_shapes;
	other_shapes.clear();
	return temp;
}

