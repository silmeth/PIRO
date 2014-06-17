/*
 * fingering.cpp
 *
 *  Created on: 3 cze 2014
 *      Author: silmeth
 */

#include "fingering.h"

Mat skinHue(const Mat & image) {
	vector<Point2f> corners;
	Mat result;
	Rect rectangle_mask(2, 2, image.cols-4, image.rows-4);
	Mat cropped = image(rectangle_mask);
	cropped.copyTo(result);

	image.copyTo(result);
	/// Convert result to HSV
	cvtColor(result, result, CV_BGR2HSV);

	Mat chans[3]; // 3 channels
	split(result, chans);
	Mat & hue = chans[0];
	Mat & sat = chans[1];

	Mat dst1;
	Mat dst2;
	Mat dst3;

	inRange(hue, Scalar(0), Scalar(13), dst1);
	inRange(hue, Scalar(175), Scalar(180), dst2);
	inRange(sat, Scalar(26), Scalar(229), dst3);

	hue = min(max(dst1, dst2), dst3);
//	hue = min(dst1, dst3);

//	cvtColor(hue, hue, CV_GRAY2BGR);
	return hue;
}

vector<vector<Point> > findFingerContours(const Mat & cam_mat) {
	Mat hue(skinHue(cam_mat));
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	int erosion_type = 2;
	int erosion_size = 1;
	Mat element = getStructuringElement(erosion_type,
										   Size(2*erosion_size + 1, 2*erosion_size+1),
										   Point(erosion_size, erosion_size));
	erode(hue, hue, element);

	dilate(hue, hue, element);

	findContours(hue, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE,
			Point(0, 0));
	return contours;
}

vector<Point> findFingerContour(const Mat & cam_mat) {
	vector<vector<Point> > finger_contours = findFingerContours(cam_mat);
	if (finger_contours.size() > 0) {
		double largestArea = 0.;
		unsigned int largest = 0;
		for(unsigned int i = 0; i < finger_contours.size(); i++) {
			double a = contourArea(finger_contours[i]);
			if(largestArea < a) {
				largestArea = a;
				largest = i;
			}
		}
		if(finger_contours.size() > 10) return finger_contours[largest];
	}
	return vector<Point>();
}

Point findFingerTip(const Mat & cam_mat) {
	const unsigned int pt_len = 30;

	vector<Point> contour = findFingerContour(cam_mat);
	vector<Point> polygon = contour;

//	if(contour.size() > 0) approxPolyDP(contour, polygon, 18, false);

	unsigned int sz = polygon.size();

	if(sz >= 2*pt_len + 1) {
		Mat drawing;

		double maxcosa = 0.; // maximum of cosine between two point and vertex in third one
		unsigned int ft_idx = 0; // index of fingertip's point
		for(unsigned int i = 0; i < sz; i++) {
			unsigned int prev = ((i-pt_len)%sz+sz)%sz;
			unsigned int next = (i+pt_len)%sz;
			// Here be dragons (i.e. calculating sqrs of lengths between points of the polygon)
			double a = (polygon[i].x-polygon[prev].x)*(polygon[i].x-polygon[prev].x) +
					(polygon[i].y-polygon[prev].y)*(polygon[i].y-polygon[prev].y);
			double b = (polygon[i].x-polygon[next].x)*(polygon[i].x-polygon[next].x) +
					(polygon[i].y-polygon[next].y)*(polygon[i].y-polygon[next].y);
			double c = (polygon[next].x-polygon[prev].x)*(polygon[next].x-polygon[prev].x) +
					(polygon[next].y-polygon[prev].y)*(polygon[next].y-polygon[prev].y);

			// Based on cosine theorem
			double cosa = (a+b+c)/(2*sqrt(a*b));
			if(maxcosa < cosa) {
				maxcosa = cosa;
				ft_idx = i;
			}
		}

		return polygon[ft_idx];
	}
	return Point(0, 0);
}


