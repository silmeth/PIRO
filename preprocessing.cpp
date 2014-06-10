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
		avg_corners.push_back(p);
	}
	for (int i=0; i<3; i++){
		last_corners.push_back(temp_vec);
	}
}

/*
 * src - BGR Mat
 * returns 1 for page, 0 for surrounding
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
	imshow("VIDEO", temp3);

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

Mat * Preprocessing:: getFinger(const Mat & src){
	return 0;
}

vector<vector<Point> > Preprocessing:: getShapes(const Mat & src){
	return 0;
}

