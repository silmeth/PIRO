#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/contrib/contrib.hpp"

#include "tests.h"
#include "shape_finder.h"
#include "preprocessing.h"
#include "straightener.h"
#include "fingering.h"

using namespace std;

int main(int argc, const char** argv) {
    Preprocessing preproc;
    Straightener straight(640, 480);
    bool shapes_found = false;
    vector<vector<Point> > triangles, rectangles, other_shapes, temp_shapes, circles;
    vector<Point> finger_contour;
    Point finger_tip;
    // Colors
    Scalar finger_contour_color( 255, 0, 0);
    Scalar finger_tip_color(rand()&255, rand()&255, rand()&255);
    Scalar triangle_color(0, 255, 0);
    Scalar rectangle_color(0, 0, 255);
    Scalar other_shape_color(255, 0, 0);
    Scalar circle_color(0, 140, 255);
    int c;
    VideoCapture cv_cap(1); //    CvCapture* cv_cap = cvCaptureFromCAM(1);
    namedWindow("Video", WINDOW_OPENGL); // create window
    Mat finger;
    int frames_with_shapes = 0;
    while(true) {
    	Mat cam_mat;
    	cv_cap >> cam_mat;
    	if(cam_mat.rows > 0 && cam_mat.cols > 0) {
            Mat str_cam_mat;
            // zwraca narożniki
            vector<Point> corners = preproc.getCorners2(cam_mat);
            if(corners.size() == 4) {
                straight.setCorners(corners);
                if(straight.straightenImage(cam_mat, str_cam_mat)) {
                    if(!shapes_found){
                    	preproc.getShapes(str_cam_mat);
						triangles = preproc.getTriangles();
						rectangles = preproc.getRectangles();
						circles = preproc.getCircles();
						other_shapes = preproc.getOtherShapes();
						if(triangles.size() == 4
								&& rectangles.size() == 6
								&& other_shapes.size() == 2
								&& circles.size() == 8){
							frames_with_shapes += 1;
						}
						else{
							frames_with_shapes = 0;
						}
						if(frames_with_shapes == 3){
							shapes_found = true;
						}
						Mat merged = preproc.mergeMatrixes(cam_mat, str_cam_mat);
						if(!merged.empty()){
							imshow("Video", merged);
						}
                    }
                    if(shapes_found) {
                        Mat drawing = str_cam_mat.clone();
                        finger_tip = findFingerTip(str_cam_mat);
                        finger_contour = findFingerContour(str_cam_mat);
                        drawContours(drawing, triangles, -1, triangle_color, 2);
						drawContours(drawing, rectangles, -1, rectangle_color, 2);
						drawContours(drawing, other_shapes, -1, other_shape_color, 2);
						drawContours(drawing, circles, -1, circle_color, 2);
                        if(finger_contour.size() > 0) {
                            vector<vector<Point> > finger_contours_tmp;
                            finger_contours_tmp.push_back(finger_contour);
                            drawContours(drawing, finger_contours_tmp, 0, finger_contour_color);
                            circle(drawing, finger_tip, 5, finger_tip_color, 3);
                        }
                        Mat merged = preproc.mergeMatrixes(cam_mat, drawing);
                        if(!merged.empty()){
                        	imshow("Video", merged);
                        }
                    }
                }
            }
            c = cvWaitKey(10); // wait 10 ms or for key stroke
            if(c == 27) {
                break; // if ESC, break and quit
            }
        }
        c = cvWaitKey(10); // wait 10 ms or for key stroke
        if(c == 27)
            break; // if ESC, break and quit
    }
    /* clean up */
    cv_cap.release();
    destroyAllWindows();
    waitKey(0);
    return 0;
}
