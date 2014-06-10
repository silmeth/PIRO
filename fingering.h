/*
 * fingering.h
 *
 *  Created on: 3 cze 2014
 *      Author: silmeth
 */

#ifndef FINGERING_H_
#define FINGERING_H_

#include "straightener.h"

#include <vector>
using namespace std;

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/contrib/contrib.hpp"
using namespace cv;

Mat skinHue(const Mat & image);

vector<vector<Point> > findFingerContours(const Mat & cam_mat);

vector<Point> findFingerContour(const Mat & cam_mat);

Point findFingerTip(const Mat & cam_mat);


#endif /* FINGERING_H_ */
