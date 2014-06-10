/*
 * preprocessing.h
 *
 *  Created on: June 03, 2014
 *      Author: jacek
 */
#ifndef PREPROCESSING_H_
#define PREPROCESSING_H_

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/contrib/contrib.hpp"

#include "straightener.h"

using namespace std;
using namespace cv;

class Preprocessing{
public:
	Preprocessing(void);
	vector<Point> getCorners(const Mat & src);
	vector<vector<Point> > getShapes(const Mat & src);
	void addNewCorners(vector<Point>);
	void avgCorners();
	vector<Point> avg_corners;
private:
	Straightener straight;
	vector<vector<Point> > last_corners;
};



#endif
