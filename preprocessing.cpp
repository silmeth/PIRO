/*
 * preprocessing.cpp
 *
 *  Created on: Jun 3, 2014
 *      Author: jacek
 */

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
		/// Average last 3 sets of corners
		avgCorners();
		/// Display avg set of corners
		//return last_corners[2];
		return avg_corners;
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
	vector<Point> triangle_cnt;
	Mat triangle_mat = imread("./img/triangle.png");
	Mat gray;
	cvtColor(src, gray, CV_BGR2GRAY);
	blur(gray, gray, Size(5,5));
	vector<vector<Point> > temp_shapes;
	return temp_shapes;
}

