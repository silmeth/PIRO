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

using namespace std;
using namespace cv;

class Preprocessing{
public:
	Preprocessing(void);
	Mat getPage(const Mat & src);
	Mat * getFinger(const Mat & src);
	Mat * getShapes(const Mat & src);
	void addNewCorners(vector<Point>);
	void getAvgCorners();
private:
	vector<vector<Point> > last_corners;
	vector<Point> avg_corners;

};



#endif
