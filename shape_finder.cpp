#include "shape_finder.h"

vector<Point2f> corners_old;
int refresh_corners = 0;

void camera_raw_display(int num) {
	int c;
	IplImage* color_img;
	CvCapture* cv_cap = cvCaptureFromCAM(num);
	cvNamedWindow("Video",0); // create window
	for(;;) {
		color_img = cvQueryFrame(cv_cap); // get frame
		if(color_img != 0)
			cvShowImage("Video", color_img); // show frame
		c = cvWaitKey(10); // wait 10 ms or for key stroke
		if(c == 27)
			break; // if ESC, break and quit
	}
	/* clean up */
	cvReleaseCapture( &cv_cap );
	cvDestroyWindow("Video");
}

void camera_straighten_display(int num, char* window_name) {
	int c;
	IplImage* color_img;
	CvCapture* cv_cap = cvCaptureFromCAM(num);
	cvNamedWindow("Video", 0); // create window
	for(;;) {
		color_img = cvQueryFrame(cv_cap); // get frame
		if(color_img != 0) {
			Mat cam_mat(color_img);
			Mat result;
			cam_mat.copyTo(result);
			if ( straighten(cam_mat, result, 450, 300) == true ) {
				imshow("Video", result); // show frame
			}
		}
		c = cvWaitKey(10); // wait 10 ms or for key stroke
		if(c == 27)
			break; // if ESC, break and quit
	}
	/* clean up */
	cvReleaseCapture( &cv_cap );
	cvDestroyWindow("Video");
}

void camera_contours_display(int num) {
	int c;
		IplImage* color_img;
		CvCapture* cv_cap = cvCaptureFromCAM(num);
		cvNamedWindow("Video", 0); // create window
		resizeWindow("Video", 700,700);
		for(;;) {
			color_img = cvQueryFrame(cv_cap); // get frame
			if(color_img != 0) {
				Mat cam_mat(color_img);
				Mat result;
				cam_mat.copyTo(result);
				if (straighten(cam_mat, result, 423, 300) == true ) {
					///Apply blur
					blur(result, result, Size(3,3));
					///Apply Canny to destination Matrix
					Canny(result, result, 50, 50, 3);
					/// Vectors for storing contours
					vector<vector<Point> > contours; //contours of the paper sheet
					vector<vector<Point> > approx_contours; //approx contours of the paper sheet
					vector<Vec4i> hierarchy;
					/// Cut 20 px from each side to avoid paper borders detection
					result = result(Rect(10, 10, result.cols-20, result.rows-20));
					findContours( result, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE, Point(0, 0) );
					/// Draw contours
					Mat drawing = Mat::zeros( result.size(), CV_8UC3 );
					/// https://github.com/Itseez/opencv/blob/master/samples/cpp/contours2.cpp
//					approx_contours.resize(contours.size());
					for(unsigned int i = 0; i < contours.size(); i++) {
						/// Area of more than 20 and no parent
						if(contourArea(contours[i]) > 20 && hierarchy[i][3] == -1) {
							vector<Point> tmp_contour;
							approxPolyDP(Mat(contours[i]), tmp_contour, 5, true);
							approx_contours.push_back(tmp_contour);
						}
					}
					for(unsigned int i=0; i < approx_contours.size(); i++) {
						Scalar color;
						if(approx_contours[i].size() == 4) {
							color = Scalar( 255, 255, 255);
							drawContours( drawing, approx_contours, i, color, 1, 8, NULL, 0, Point() );
						}
						else {
							color = Scalar( 0, 255, 0);
							drawContours( drawing, approx_contours, i, color, 1, 8, NULL, 0, Point() );
						}
					}
					imshow("Video", drawing);
				}
			}
			c = cvWaitKey(10); // wait 10 ms or for key stroke
			if(c == 27)

				break; // if ESC, break and quit
		}
		/* clean up */
		cvReleaseCapture( &cv_cap );
		cvDestroyWindow("Video");
}

/*
 * Copied from http://opencv-code.com/tutorials/automatic-perspective-correction-for-quadrilateral-objects/
 */
void sortCorners(vector<Point2f>& corners, Point2f center) {
    vector<Point2f> top, bot;

    for ( unsigned int i = 0; i < corners.size(); i++ ) {
        if (corners[i].y < center.y && top.size() < 2)
            top.push_back(corners[i]);
        else
            bot.push_back(corners[i]);
    }

    Point2f tl = top[0].x > top[1].x ? top[1] : top[0];
    Point2f tr = top[0].x > top[1].x ? top[0] : top[1];
    Point2f bl = bot[0].x > bot[1].x ? bot[1] : bot[0];
    Point2f br = bot[0].x > bot[1].x ? bot[0] : bot[1];

    corners.clear();
    corners.push_back(tl);
    corners.push_back(tr);
    corners.push_back(br);
    corners.push_back(bl);
}

/*
 * Copied from http://opencv-code.com/tutorials/automatic-perspective-correction-for-quadrilateral-objects/
 */
bool straighten(Mat &src, Mat &dst, unsigned int rows, unsigned int cols) {
	vector<cv::Vec4i> slines;
	vector<par_line> par_lines;
	vector<par_line> borders;
	bool new_corners = false;
	vector<Point2f> corners;
	vector<Point2f> quad_pts;

	Mat temp;
	blur(src, temp, Size(5,5));
	Canny(temp, temp, 50, 50, 3);
	int erosion_type = 2;
	int erosion_size = 1;
	Mat element = getStructuringElement( erosion_type,
										   Size( 2*erosion_size + 1, 2*erosion_size+1 ),
										   Point( erosion_size, erosion_size ) );
	dilate(temp, temp, element);
	HoughLinesP(temp, slines, 1, CV_PI/360, 120,100, 10);

	if ( slines.size() < 4 ) {
		///cout << "Hough: Znaleziono mniej niż 4 linie.";
		new_corners = false;
	}
	else {
		for( unsigned int i = 0; i < slines.size(); i++ ) {
			Vec4i l = slines[i];
			par_line tmp_line;
			/// Pionowa linia - b na dużą wartość
			if( abs(l[2]-l[0]) < 2 ){
				// TODO znak powinien zależeć od atana
				tmp_line.b = 1.e15;
			}
			else {
				tmp_line.b = l[1] - (double)(l[3]-l[1])/((double)(l[2]-l[0]))*l[0];
			}
			tmp_line.atana = atan2((double)(l[3]-l[1]),((double)(l[2]-l[0])));
			tmp_line.len = sqrt(pow( (double)(l[0]-l[2]), 2.0 ) + pow( (double)(l[1]-l[3]), 2.0 ));
			par_lines.push_back(tmp_line);
		}

		/// Uśrednione linie będące krawędziami kartki
		/// Dla każdej linii znajdź taką, która mieści się w zakresie +- 10 stopni
		/// Pierwsza linia trafia od razu
		borders.push_back(par_lines[0]);
		for( unsigned int i = 1; i < par_lines.size(); i++ ) {
			bool found_similiar = false;
			for ( unsigned int j = 0; j < borders.size(); j++ ) {
				/// Nowy odcinek podobny do któregoś z istniejących
				if ( abs(abs(par_lines[i].atana) - abs(borders[j].atana)) < 10.0*3.14159/180.0
					&& abs(par_lines[i].b - borders[j].b) < 150.0 ) {
					/// Nowa wartość jako średnia ważona
					borders[j].atana = (borders[j].atana*borders[j].len
										   + par_lines[i].atana*par_lines[i].len)
										   / (borders[j].len + par_lines[i].len);
					borders[j].b = (borders[j].b*borders[j].len
														   + par_lines[i].b*par_lines[i].len)
														   / (borders[j].len + par_lines[i].len);
					/// Zapisz nową długość uśrednionej prostej
					borders[j].len = borders[j].len + par_lines[i].len;
					found_similiar = true;
				}
			}
			/// Jeżeli żaden element nie był podobny, dodaj nową krawędź
			if ( !found_similiar ) {
				borders.push_back(par_lines[i]);
			}
		}
		if ( borders.size() < 4 ) {
			///cout << "Zbudowano mniej niż 3 boki obwiedni.";
			new_corners = false;
		}
		else{
			/// Znajdź narożniki
			for (unsigned int i = 0; i < borders.size(); i++){
				for (unsigned int j = i+1; j < borders.size(); j++){
					/// Znajdź przecięcie między nierównoległymi do siebie brzegami kartki
					if( abs(abs(borders[i].atana)-abs(borders[j].atana)) > 45.0*3.14159/180.0 ) {
						Point2f p;
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
				/// Oblicz środek masy
				Point2f center(0,0);
				for ( unsigned int i = 0; i < corners.size(); i++ ) {
					center += corners[i];
				}
				center *= (1. / corners.size());
				/// Posortuj narożniki
				sortCorners(corners, center);
				new_corners = true;
			}
		}
	}
	/// Jeżeli to pierwszy przebieg, skopiuj narożniki
	if( corners_old.size() == 0 ) {
		corners_old = corners;
	}
	/// Jeżeli są nowe narożniki
	if( new_corners ) {
		/// Co 20 klatkę odświeżaj narożniki, aby uniknąć przekrzywienia po obrocie i powrocie
		if( refresh_corners > 20 ){
			corners_old = corners;
			refresh_corners = 0;
		}
		else {
			refresh_corners += 1;
			bool close_corner_found [4];
			for( int i = 0; i < 4; i++ ) {
				close_corner_found[i] = false;
				Point2f c = corners_old[i];
				for( int j = 0; j < 4; j++ ) {
					Point2f k = corners[j];
					if( abs(k.y - c.y) < 150 && abs(k.x - c.x) < 150 ) {
						close_corner_found[i] = true;
					}
				}
			}
			if ( close_corner_found[0] && close_corner_found[1] && close_corner_found[2] && close_corner_found[3] ) {
				corners_old = corners;
			}
		}
	}
	if( corners_old.size() == 4 ) {
		// Define the destination image
		dst = Mat::zeros(cols, rows, CV_8UC3);
		// Corners of the destination image
		quad_pts.push_back(Point2f(0, 0));
		quad_pts.push_back(Point2f(dst.cols, 0));
		quad_pts.push_back(Point2f(dst.cols, dst.rows));
		quad_pts.push_back(Point2f(0, dst.rows));
		// Get transformation matrix
		Mat transmtx = getPerspectiveTransform(corners_old, quad_pts);
		// Apply perspective transformation
		warpPerspective(src, dst, transmtx, dst.size());
		return true;
	}
	else {
		return false;
	}
}


