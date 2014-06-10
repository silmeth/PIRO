/*
 * straightener.cpp
 *
 *  Created on: 3 cze 2014
 *      Author: silmeth
 */

#include "straightener.h"

void Straightener::sortCorners() {
    vector<Point2f> top, bot;

	/// Find center of mass
	Point2f center(0,0);
	for(unsigned int i = 0; i < corners.size(); i++) {
		center += Point2f(corners[i].x, corners[i].y);
	}
	center *= (1. / corners.size());

	int num_top = -1;
	int num_bot = -1;
	int num_corners = -1;
    for(unsigned int i = 0; i < corners.size(); i++) {
        if ((float)(corners[i].y) <= center.y && top.size() < 2)
            top.push_back(corners[i]);
        else
            bot.push_back(corners[i]);
    }
    num_top = top.size();
    num_bot = bot.size();
    num_corners = corners.size();

    Point tl = top[0].x > top[1].x ? top[1] : top[0];
    Point tr = top[0].x > top[1].x ? top[0] : top[1];
    Point bl = bot[0].x > bot[1].x ? bot[1] : bot[0];
    Point br = bot[0].x > bot[1].x ? bot[0] : bot[1];

    corners.clear();
    corners.push_back(tl);
    corners.push_back(tr);
    corners.push_back(br);
    corners.push_back(bl);
}

Straightener::Straightener(const Mat & src, unsigned int w, unsigned int h) :
		width(w), height(h), refresh_corners(0) {
	findCorners(src);
}

Straightener::Straightener(unsigned int w, unsigned int h) :
	width(w), height(h), refresh_corners(0) { }

vector<Point> Straightener::getCorners() {
	return corners_old;
}

void Straightener::setCorners(const vector<Point> & new_corners) {
	corners = new_corners;
	sortCorners();
	corners_old = corners;
}

Mat Straightener::getTransMatrix() {
	return trans_mat;
}

bool Straightener::findCorners(const Mat & src) {
	vector<cv::Vec4i> slines;
	vector<par_line> par_lines;
	vector<par_line> borders;
	bool new_corners = false;
	Mat temp;
	blur(src, temp, Size(5,5));
	Canny(temp, temp, 100, 100, 3);
	int erosion_type = 1;
	int erosion_size = 1;
	Mat element = getStructuringElement(erosion_type,
										Size(2*erosion_size + 1, 2*erosion_size+1),
										Point(erosion_size, erosion_size));
	dilate(temp, temp, element);
	HoughLinesP(temp, slines, 1, CV_PI/360, 120, 100, 10);

	if (slines.size() < 4) {
		new_corners = false;
	}
	else {
		for(unsigned int i = 0; i < slines.size(); i++) {
			Vec4i l = slines[i];
			par_line tmp_line;
			/// Vertical line - b equals infinity
			if( abs(l[2]-l[0]) == 0 ){
				// sign (should be) OK
				tmp_line.b = -(l[3]-l[1])/abs(l[3]-l[1])*INF;
			}
			else {
				tmp_line.b = l[1] - (double)(l[3]-l[1])/((double)(l[2]-l[0]))*l[0];
			}
			tmp_line.atana = atan2((double)(l[3]-l[1]),((double)(l[2]-l[0])));
			tmp_line.len = sqrt(pow( (double)(l[0]-l[2]), 2.0 ) + pow((double)(l[1]-l[3]), 2.0));
			par_lines.push_back(tmp_line);
		}

		/// Averaged line that are paper-sheet edges
		/// For every line find another one that is tilted similarly, in range +- 10 degrees
		/// First line added at once
		borders.push_back(par_lines[0]);
		for( unsigned int i = 1; i < par_lines.size(); i++ ) {
			bool found_similiar = false;
			for ( unsigned int j = 0; j < borders.size(); j++ ) {
				/// Check if new segment similar to one of the previous ones
				if (abs(abs(par_lines[i].atana) - abs(borders[j].atana)) < 10.0*PI/180.0
					&& abs(par_lines[i].b - borders[j].b) < 150.0 ) {
					/// New value as average weighted by length
					borders[j].atana = (borders[j].atana*borders[j].len
										   + par_lines[i].atana*par_lines[i].len)
										   / (borders[j].len + par_lines[i].len);
					borders[j].b = (borders[j].b*borders[j].len
									+ par_lines[i].b*par_lines[i].len)
									/ (borders[j].len + par_lines[i].len);
					/// Save the length of new border/edge
					borders[j].len = borders[j].len + par_lines[i].len;
					found_similiar = true;
				}
			}
			/// If none of the edges was similar, add as new
			if ( !found_similiar ) {
				borders.push_back(par_lines[i]);
			}
		}
		if ( borders.size() < 4 ) {
			new_corners = false;
		}
		else {
			// Remove the shortest segments
			while(borders.size() > 4) {
				unsigned char i = 0;
				for(unsigned char j = 0; j < borders.size(); j++) {
					if(borders[i].len > borders[j].len) {
						i = j;
					}
				}
				borders.erase(borders.begin()+i);
			}
			/// Find corners
			for (unsigned int i = 0; i < borders.size(); i++){
				for (unsigned int j = i+1; j < borders.size(); j++){
					/// Find intersection between perpendicular edges of paper-sheet
					if(abs(abs(borders[i].atana)-abs(borders[j].atana)) > 45.0*PI/180.0 ) {
						Point p;
						p.x = (borders[i].b - borders[j].b) /
								((tan(borders[j].atana) - tan(borders[i].atana)));
						p.y = p.x * tan(borders[i].atana) + borders[i].b;
						corners.push_back(p);
					}
				}
			}
			if ( corners.size() < 4 ) {
				new_corners = false;
			}
			else {
				/// Sort corners
				sortCorners();
				new_corners = true;
			}
		}
	}
	/// Copy corners if initialization
	if(corners_old.size() == 0) {
		corners_old = corners;
	}
	/// If new corners found
	if(new_corners) {
		/// Every 20 frames refresh corners no matter what
		if(refresh_corners > 20){
			corners_old = corners;
			refresh_corners = 0;
		}
		else {
			refresh_corners += 1;
			bool close_corner_found [4];
			for(int i = 0; i < 4; i++) {
				close_corner_found[i] = false;
				Point c = corners_old[i];
				for( int j = 0; j < 4; j++ ) {
					Point k = corners[j];
					if(abs(k.y - c.y) < 100 && abs(k.x - c.x) < 100) {
						close_corner_found[i] = true;
					}
				}
			}
			if(close_corner_found[0] && close_corner_found[1] &&
			   close_corner_found[2] && close_corner_found[3]) {
				corners_old = corners;
			}
		}
	}
	if(corners_old.size() == 4) {
		return true;
	}
	else {
		return false;
	}
}

bool Straightener::findTransMatrix(const Mat & src, bool newCorners) {
	const unsigned int rows = height;
	const unsigned int cols = width;
	vector<Point2f> quad_pts;
	vector<Point2f> corners_old2f;

	if(newCorners) {
		if(!findCorners(src)) {
			return false;
		}
	}
	// Corners of the destination image
	quad_pts.push_back(Point2f(0, 0));
	quad_pts.push_back(Point2f(cols, 0));
	quad_pts.push_back(Point2f(cols, rows));
	quad_pts.push_back(Point2f(0, rows));

	for(int i=0; i<4; i++){
		corners_old2f.push_back(Point2f(corners_old[i].x, corners_old[i].y));
	}
	trans_mat = getPerspectiveTransform(corners_old2f, quad_pts);
	return true;
}

bool Straightener::doAll(const Mat & src, Mat & dst) {
	const unsigned int rows = height;
	const unsigned int cols = width;
	if(findTransMatrix(src, true)) {
		dst = Mat::zeros(rows, cols, CV_8UC3);
		warpPerspective(src, dst, trans_mat, dst.size());
		return true;
	}
	return false;
}

bool Straightener::straightenImage(const Mat & src, Mat & dst) {
	const unsigned int rows = height;
	const unsigned int cols = width;
	if(findTransMatrix(src, false)) {
		dst = Mat::zeros(rows, cols, CV_8UC3);
		warpPerspective(src, dst, trans_mat, dst.size());
		return true;
	}
	return false;
}
