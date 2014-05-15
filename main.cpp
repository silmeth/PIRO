#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/contrib/contrib.hpp"


#include <iostream>
#include <stdio.h>

#include <cmath>

using namespace std;
using namespace cv;

Mat src, src_gray;
Mat dst, detected_edges;

int edgeThresh = 1;
int canny_low_threshold = 50;
int max_canny_low_threshold = 100;
int ratio = 3;
int kernel_size = 3;
char* window_name = "Edge Map";

int min_adapt_max_val = 0;
int min_adapt_block_size = 1;

int canny_kernel_size = 1;
int max_canny_kernel_size = 200;

std::vector<cv::Vec4i> hough_lines; // to find contours?
int hough_rho = 1;
int hough_theta = 1;
int hough_threshold = 20;
int hough_min_line_length = 30;
int hough_max_line_gap = 10;

vector<vector<Point> > contours; //contours of the paper sheet
vector<Vec4i> hierarchy;

vector<vector<Point> > approx_contours;

// parameterized line
typedef struct {
	double atana; // arc tangent a
	double b; // parameter b
	double len;
} par_line;

void camera_raw_display() {
	int c;
	IplImage* color_img;
	CvCapture* cv_cap = cvCaptureFromCAM(1);
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

void canny_blur_filter(Mat &src, Mat &dst, int kernel_size, int low_treshold, int ratio){
	// Create destination
	dst = Scalar::all(0);
	// Stores detected edges
	Mat detected_edges;
	// Copy source do detected edges
	src.copyTo(detected_edges);
	blur(detected_edges, detected_edges, Size(3,3));
	// Apply Canny to destination Matrix
	Canny(detected_edges, detected_edges, low_treshold, low_treshold*ratio, kernel_size);
	src.copyTo(dst, detected_edges);
}

void canny_window(int, void*){
	blur(src_gray, dst, Size(3,3));
	Canny(dst, dst, 100, 100, 3);
	imshow( window_name, dst );
}

void adaptive_treshold(Mat &src, Mat &dst, int max_value, int block_size, double threshold_constant){
	src.copyTo(dst);
	blur(dst, dst, Size(3,3));
	adaptiveThreshold(src, dst, (double)max_value, ADAPTIVE_THRESH_MEAN_C,
			THRESH_BINARY, block_size, threshold_constant);
}

void adaptive_threshold_window(int, void*) {
	adaptive_treshold(src_gray, dst, min_adapt_max_val, min_adapt_block_size*2 + 1, 0.0);
	imshow(window_name, dst);
}

void hough_canny_window(int, void*) { // find edges of paper sheet
	Mat temp;

	canny_blur_filter(src_gray, temp, 3, 50, 3);
	HoughLinesP(temp, hough_lines, hough_rho, CV_PI/1800*hough_theta, hough_threshold,
				(double)hough_min_line_length, (double)hough_max_line_gap);
	temp.copyTo(dst);
	for( size_t i = 0; i < hough_lines.size(); i++ ) {
	    Vec4i l = hough_lines[i];
	    line( dst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, CV_AA);
	  }
	imshow(window_name, dst);
}

void contours_window(int, void*) {
	Mat temp;
	canny_blur_filter(src_gray, temp, 3, 50, 3);
	findContours( temp, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
	/// Draw contours
	Mat drawing = Mat::zeros( temp.size(), CV_8UC3 );
	// Z tutka https://github.com/Itseez/opencv/blob/master/samples/cpp/contours2.cpp
	approx_contours.resize(contours.size());
	for(unsigned int i = 0; i < contours.size(); i++){
		approxPolyDP(Mat(contours[i]), approx_contours[i], 5, true);
	}
	// Plotujemy !!!!
	for( unsigned int i=0; i< approx_contours.size(); i++ ) {
		Scalar color = Scalar( 255, 0, 0);
		drawContours( drawing, contours, i, color, 1, 8, hierarchy, 0, Point() );
		color = Scalar( 0, 255, 0);
		drawContours( drawing, approx_contours, i, color, 1, 8, hierarchy, 0, Point() );
	}
	imshow(window_name, drawing);
}

/*
 * Copied from http://opencv-code.com/tutorials/automatic-perspective-correction-for-quadrilateral-objects/
 */
void sortCorners(std::vector<cv::Point2f>& corners, cv::Point2f center)
{
    std::vector<cv::Point2f> top, bot;

    for ( unsigned int i = 0; i < corners.size(); i++ ) {
        if (corners[i].y < center.y)
            top.push_back(corners[i]);
        else
            bot.push_back(corners[i]);
    }

    cv::Point2f tl = top[0].x > top[1].x ? top[1] : top[0];
    cv::Point2f tr = top[0].x > top[1].x ? top[0] : top[1];
    cv::Point2f bl = bot[0].x > bot[1].x ? bot[1] : bot[0];
    cv::Point2f br = bot[0].x > bot[1].x ? bot[0] : bot[1];

    corners.clear();
    corners.push_back(tl);
    corners.push_back(tr);
    corners.push_back(br);
    corners.push_back(bl);
}

void straighten(Mat &src, Mat &dst) {
	std::vector<cv::Vec4i> slines;
	Mat temp;
	blur(src, temp, Size(3,3));
	Canny(temp, temp, 100, 100, 3);
	HoughLinesP(temp, slines, 1, CV_PI/360, 65, 80, 10);
	Mat drawing = Mat::zeros( temp.size(), CV_8UC3 );
	drawing = Scalar::all(0);

	vector<par_line> par_lines;

	for( unsigned int i = 0; i < slines.size(); i++ ) {
		/// Wyświetla linie obliczone przez transformatę Hougha
		//line( drawing, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, CV_AA);
		Vec4i l = slines[i];
		par_line tmp_line;
		tmp_line.atana = atan((double)(l[3]-l[1])/((double)(l[2]-l[0])));
		tmp_line.b = l[1] - (double)(l[3]-l[1])/((double)(l[2]-l[0]))*l[0];
		tmp_line.len = sqrt(pow( (double)(l[0]-l[2]), 2.0 ) + pow( (double)(l[1]-l[3]), 2.0 ));
		//cout << "b" << i << " = " << tmp_line.b << "\natan(a)" << i << " = " << tmp_line.atana*180/3.14159
		//     << "\nlen = " << tmp_line.len << endl;
	    par_lines.push_back(tmp_line);
	  }
	cout << slines.size();
	/// Uśrednione linie będące krawędziami kartki
	vector<par_line> paper_borders;
	// Dla każdej linii znajdź taką, która mieści się w zakresie +- 10 stopni
	// Pierwsza linia trafia od razu
	paper_borders.push_back(par_lines[0]);
	for( unsigned int i = 1; i < par_lines.size(); i++ ) {
		bool found_similiar = false;
		for (unsigned int j = 0; j < paper_borders.size(); j++){
			/// Nowy odcinek podobny do któregoś z istniejących
			if ( abs(par_lines[i].atana - paper_borders[j].atana) < 10.0*3.14159/180.0
				&& abs(par_lines[i].b - paper_borders[j].b) < 150.0 ) {
				/// Nowa wartość jako średnia ważona
				paper_borders[j].atana = (paper_borders[j].atana*paper_borders[j].len
									   + par_lines[i].atana*par_lines[i].len)
									   / (paper_borders[j].len + par_lines[i].len);
				paper_borders[j].b = (paper_borders[j].b*paper_borders[j].len
													   + par_lines[i].b*par_lines[i].len)
													   / (paper_borders[j].len + par_lines[i].len);
				/// Zapisz nową długość uśrednionej prostej
				paper_borders[j].len = paper_borders[j].len + par_lines[i].len;
				found_similiar = true;
			}
		}
		/// Jeżeli żaden element nie był podobny, dodaj nową krawędź
		if ( !found_similiar ) {
			paper_borders.push_back(par_lines[i]);
		}
	}
	/// Wypisz nowo obliczone krawędzie kartki
	for (unsigned int i = 0; i < paper_borders.size(); i++){
		cout << "b" << i << " = " << paper_borders[i].b << "\natan(a)" << i << " = " << paper_borders[i].atana*180/3.14159
				     << "\nlen = " << paper_borders[i].len << endl;
	}
	/// Wyświetl nowo obliczone krawędzie kartki
	for (unsigned int i = 0; i < paper_borders.size(); i++){
		/// Oblicz punktu końcowe prostych poziomych
		/// TODO Zmień ze stałych wartości na zależne od rozmiaru obrazu
		if( abs(paper_borders[i].atana) < 20.0*3.14159/180.0 ){
			line( src_gray, Point(5, 5*tan(paper_borders[i].atana)+paper_borders[i].b),
						   Point(635, 635*tan(paper_borders[i].atana)+paper_borders[i].b),
					       Scalar(255,255,255), 3, CV_AA);
		}
		/// Oblicz punkty końconwe prostych pionowych
		else {
			line( src_gray, Point((5-paper_borders[i].b) / tan(paper_borders[i].atana), 5),
						   Point((475-paper_borders[i].b) / tan(paper_borders[i].atana), 475),
						   Scalar(255,255,255), 3, CV_AA);
		}
		cout << 5*tan(paper_borders[i].atana)+paper_borders[i].b << endl;
	}

	/// Znajdź narożniki
	std::vector<cv::Point2f> corners;
	for (unsigned int i = 0; i < paper_borders.size(); i++){
		for (unsigned int j = i+1; j < paper_borders.size(); j++){
			/// Znajdź przecięcie między nierównoległymi do siebie brzegami kartki
			if( abs(abs(paper_borders[i].atana)-abs(paper_borders[j].atana)) > 45.0*3.14159/180.0 ) {
				Point2f p;
				p.x = (paper_borders[i].b - paper_borders[j].b) /
						((tan(paper_borders[j].atana) - tan(paper_borders[i].atana)));
				p.y = p.x * tan(paper_borders[i].atana) + paper_borders[i].b;
				corners.push_back(p);
			}
		}
	}
	/// Wyświetl punkty przecięcia
	for (unsigned int i = 0; i < corners.size(); i++){
		circle(src_gray, corners[i], 10, Scalar(255,255,255), 2);
		//cout << "x: " << corners[i].x << "  y: " << corners[i].y << endl;
	}

	/// Get mass center
	cv::Point2f center(0,0);
	for (unsigned int i = 0; i < corners.size(); i++)
	    center += corners[i];

	center *= (1. / corners.size());
	sortCorners(corners, center);
	for (unsigned int i = 0; i < corners.size(); i++){
		cout << "x: " << corners[i].x << "  y: " << corners[i].y << endl;
	}
	// Define the destination image
	cv::Mat quad = cv::Mat::zeros(240, 320, CV_8UC3);

	// Corners of the destination image
	std::vector<cv::Point2f> quad_pts;
	quad_pts.push_back(cv::Point2f(0, 0));
	quad_pts.push_back(cv::Point2f(quad.cols, 0));
	quad_pts.push_back(cv::Point2f(quad.cols, quad.rows));
	quad_pts.push_back(cv::Point2f(0, quad.rows));

	// Get transformation matrix
	cv::Mat transmtx = cv::getPerspectiveTransform(corners, quad_pts);

	// Apply perspective transformation
	cv::warpPerspective(src_gray, quad, transmtx, quad.size());

	imshow( window_name, quad );
}

int main(int argc, const char** argv) {
	/// Load an image
	src = imread( "./img/benchmark.png" );
	if( !src.data )
	{ return -1; }
	/// Convert the image to grayscale
	cvtColor( src, src_gray, CV_BGR2GRAY );
	/// Create a window
	namedWindow( window_name, CV_WINDOW_AUTOSIZE );

	/*
	 * Trackbar Canny Threshold detector
	 */
	createTrackbar("Min Threshold:", window_name, &canny_low_threshold, max_canny_low_threshold, canny_window);
	//createTrackbar("Kernel size:", window_name, &canny_kernel_size, max_canny_kernel_size, canny_window);
	/*
	 * Trackbar Adaptive Threshold
	 */
	//createTrackbar("max_value:", window_name, &min_adapt_max_val, 255, adaptive_threshold_window);
	//createTrackbar("block_size:", window_name, &min_adapt_block_size, 500, adaptive_threshold_window);

	/*
	 * Trackbar Hough Transform
	 */
	/*
	// 1 by tutorial
	createTrackbar("rho:", window_name, &hough_rho, 5, hough_canny_window);
	// 10 by tutorial
	createTrackbar("theta:", window_name, &hough_theta, 20, hough_canny_window);
	// 70 by tutorial
	createTrackbar("threshold:", window_name, &hough_threshold, 200, hough_canny_window);
	// 30 by tutorial
	createTrackbar("min_line_length:", window_name, &hough_min_line_length, 200, hough_canny_window);
	// 10 by tutorial
	createTrackbar("max_line_gap:", window_name, &hough_max_line_gap, 200, hough_canny_window);
	createTrackbar("max_line_gap:", window_name, &hough_max_line_gap, 200, hough_canny_window);
	*/
	// contours_window(5, &min_adapt_block_size);
	straighten(src_gray, dst);
	waitKey(0);
}
