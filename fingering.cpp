/*
 * fingering.cpp
 *
 *  Created on: 3 cze 2014
 *      Author: silmeth
 */

#include "fingering.h"

Mat find_finger(int num, const Mat & trans_mat, const Mat & image,
		unsigned int rows, unsigned int cols) {
	vector<Point2f> corners;
	Mat cam_mat = image;
	Mat result = Mat::zeros(rows, cols, CV_8UC3);

	warpPerspective(cam_mat, result, trans_mat, result.size());

	/// Convert result to HSV
	cvtColor(result, result, CV_BGR2HSV);

	Mat chans[3]; // 3 channels
	split(result, chans);
	Mat & hue = chans[0];
	Mat & sat = chans[1];

	Mat dst1;
	Mat dst2;
	Mat dst3;

	inRange(hue, Scalar(0), Scalar(15), dst1);
	inRange(hue, Scalar(160), Scalar(180), dst2);
	inRange(sat, Scalar(26), Scalar(229), dst3);

	hue = min(max(dst1, dst2), dst3);

	return hue;
}




