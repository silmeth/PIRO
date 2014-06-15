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
	vector<Point> getCorners2(const Mat & src);
	/*
	 * Currently supports only matrixes of the same size!
	 */
	Mat mergeMatrixes(const Mat & mat_l, const Mat & mat_r);
	vector<vector<Point> > getShapes(const Mat & src);
	vector<vector<Point> > getTriangles();
	vector<vector<Point> > getRectangles();
	vector<vector<Point> > getCircles();
	vector<vector<Point> > getOtherShapes();
	void addNewCorners(vector<Point>);
	void avgCorners();
	vector<Point> avg_corners;
private:
	vector<vector<Point> > triangles, rectangles, other_shapes, circles;
	Straightener straight;
	vector<vector<Point> > last_corners;
};



#endif
