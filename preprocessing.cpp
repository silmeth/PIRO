/*
 * preprocessing.cpp
 *
 *  Created on: Jun 3, 2014
 *      Author: jacek
 */

#include "preprocessing.h"

using namespace std;
using namespace cv;

Preprocessing:: Preprocessing(void){
	/// Enter some points to last_corners
	Point p = Point(10, 10);
	vector <Point> temp_vec;
	for(int k=0; k<4; k++){
		temp_vec.push_back(p);
	}
	for (int i=0; i<3; i++){
		last_corners.push_back(temp_vec);
	}
}

/*
 * src - BGR Mat
 * returns 1 for page, 0 for surrounding
 */
Mat Preprocessing:: getPage(const Mat & src){
	Mat frameHSV;
	//if (src != 0){
		cvtColor(src, frameHSV, CV_BGR2HSV);
		Mat Sat = Mat(frameHSV.rows, frameHSV.cols, CV_8UC1);
		int ch[] = { 1, 0 };
		// Get Saturation from HSV
		mixChannels( &frameHSV, 1, &Sat, 1, ch, 1 );
		// Blur
		blur(Sat, Sat, Size(3,3));
		// Apply adaptive thresholding
		adaptiveThreshold(Sat, Sat, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 71, 30);

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
		/// Draw contours
		Mat drawing = Mat::zeros( Sat.size(), CV_8UC3 );
		Scalar color = Scalar( 255, 255, 255);
		Scalar color2 = Scalar(0, 255, 0);
		/// Index of the longest contour
		int ind_cnt1 = 0;
		/// Index of the second longest contour
		int ind_cnt2 = 0;
		/// Length of the longest contour
		int cnt1_len = 0;
		/// Length of second the longest contour
		int cnt2_len = 0;
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
		/// Approximate second longest contour (hopefully page)
		approxPolyDP(Mat(contours[ind_cnt2]), approx_contour, 50, true);

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
			if(is_border == false){
				last_corners[0] = last_corners[1];
				last_corners[1] = last_corners[2];
				last_corners[2] = approx_contour;
				for(int i=0; i<3; i++){
					for (int k = 0; k < 4; k++){
						circle(drawing, last_corners[i][k], 7, color, 3);
					}
				}
			}
		}

		vector<vector<Point> > temp;
		temp.push_back(approx_contour);
		drawContours( drawing, temp, 0, color2, 1, 8, NULL, 0, Point() );
		return drawing;
}

void Preprocessing:: getAvgCorners(){
	int x = 0;
	int y = 0;
	for(int i=0; i<3; i++){

	}
}

Mat * Preprocessing:: getFinger(const Mat & src){
	return 0;
}

Mat * Preprocessing:: getShapes(const Mat & src){
	return 0;
}




