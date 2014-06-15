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
    bool shapes_found = true;
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
    IplImage* color_img;
    CvCapture* cv_cap = cvCaptureFromCAM(1);
    namedWindow("Video", WINDOW_OPENGL); // create window
    Mat finger;
    while(true) {
        color_img = cvQueryFrame(cv_cap); // get frame
        if(color_img != 0) {
            Mat cam_mat(color_img);
            Mat str_cam_mat;

            // zwraca narożniki
            vector<Point> corners = preproc.getCorners2(cam_mat);
            if(corners.size() == 4) {
                straight.setCorners(corners);
                if(straight.straightenImage(cam_mat, str_cam_mat)) {
                    if(!shapes_found){
                        preproc.getShapes(str_cam_mat);
                        triangles = preproc.getTriangles();
                        if(triangles.size() > 0) {
                        	shapes_found = true;
                        }
                        imshow("Video", str_cam_mat);
                    }
                    if(shapes_found) {
                        Mat drawing = str_cam_mat.clone();
                        finger_tip = findFingerTip(str_cam_mat);
                        finger_contour = findFingerContour(str_cam_mat);
                        preproc.getShapes(str_cam_mat);
                        temp_shapes = preproc.getTriangles();
                        if(temp_shapes.size() == 4){
                        	triangles = temp_shapes;
                        }
                        temp_shapes = preproc.getRectangles();
					    if(temp_shapes.size() == 6){
					    	rectangles = temp_shapes;
					    }
					    temp_shapes = preproc.getOtherShapes();
					    if(temp_shapes.size() == 2){
					    	other_shapes = temp_shapes;
					    }
					    temp_shapes = preproc.getCircles();
						if(temp_shapes.size() == 8){
							 circles = temp_shapes;
						}
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
                        imshow("Video", merged);
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
    cvReleaseCapture(&cv_cap);
    destroyAllWindows();
    waitKey(0);
    return 0;
}
