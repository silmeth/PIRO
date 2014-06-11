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
    vector<vector<Point> > shapes;
    vector<Point> finger_contour;
    Point finger_tip;
    // Colors
    Scalar finger_contour_color( 255, 0, 0);
    Scalar finger_tip_color(rand()&255, rand()&255, rand()&255);
    Scalar shape_untouched_color(0, 255, 0);
    Scalar shape_touched_color(0, 0, 255);
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
                        shapes = preproc.getShapes(str_cam_mat);
                        if(shapes.size() > 0) {
                        	shapes_found = true;
                        }
                        imshow("Video", str_cam_mat);
                    }
                    if(shapes_found) {
                        Mat drawing = str_cam_mat;
                        finger_tip = findFingerTip(str_cam_mat);
                        finger_contour = findFingerContour(str_cam_mat);
                        if(finger_contour.size() > 0) {
                            vector<vector<Point> > finger_contours_tmp;
                            finger_contours_tmp.push_back(finger_contour);
                        	//drawContours(drawing, shapes, -1, shape_untouched_color);
                            drawContours(drawing, finger_contours_tmp, 0, finger_contour_color);
                            circle(drawing, finger_tip, 5, finger_tip_color, 3);
                        }
                        imshow("Video", drawing);
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
