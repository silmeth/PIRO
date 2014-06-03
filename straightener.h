/*
 * straightener.h
 *
 *  Created on: 3 cze 2014
 *      Author: silmeth
 */

#ifndef STRAIGHTENER_H_
#define STRAIGHTENER_H_

#define PI 3.14159
#define INF 1.e50

#include <vector>
using namespace std;

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/contrib/contrib.hpp"
using namespace cv;

typedef struct {
	double atana; // arc tangent a
	double b; // parameter b
	double len;
} par_line;

class Straightener {
	vector<Point2f> corners_old;
	vector<Point2f> corners;
	unsigned int refresh_corners;
	Mat trans_mat;
	void sortCorners();

public:
	Straightener(const Mat & src);
	Straightener();
	vector<Point2f> getCorners();
	Mat getTransMatrix();
	bool findCorners(const Mat & src);
	bool findTransMatrix(const Mat & src, unsigned int rows, unsigned int cols);
	bool straightenImage(const Mat & src, Mat & dst, unsigned int rows, unsigned int cols);
};



#endif /* STRAIGHTENER_H_ */
