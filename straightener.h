/*
 * straightener.h
 *
 *  Created on: 3 cze 2014
 *      Author: silmeth
 */

#ifndef STRAIGHTENER_H_
#define STRAIGHTENER_H_

#include <vector>
using namespace std;

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/contrib/contrib.hpp"
using namespace cv;

#include "mat_consts.h"
#include "typedefs.h"

class Straightener {
	vector<Point> corners_old;
	vector<Point> corners;
	const unsigned int width;
	const unsigned int height;
	unsigned int refresh_corners;
	Mat trans_mat;
	void sortCorners();

public:
	Straightener(const Mat & src, unsigned int w, unsigned int h);
	Straightener(unsigned int w, unsigned int h);
	vector<Point> getCorners();
	void setCorners(const vector<Point> & new_corners);
	Mat getTransMatrix();
	bool findCorners(const Mat & src);
	bool findTransMatrix(const Mat & src, bool newCorners = true);
	bool doAll(const Mat & src, Mat & dst);
	bool straightenImage(const Mat & src, Mat & dst);
};



#endif /* STRAIGHTENER_H_ */
