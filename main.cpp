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
        Straightener straight;
        int c;
        IplImage* color_img;
        CvCapture* cv_cap = cvCaptureFromCAM(1);
        cvNamedWindow("Video", 0); // create window
        Mat finger;
        for(;;) {
                color_img = cvQueryFrame(cv_cap); // get frame
                if(color_img != 0) {
                        Mat cam_mat(color_img);
                        Mat res;
                        //straight.doAll(cam_mat, res, 250, 400);
                        preproc.getPage(cam_mat);
                        straight.setCorners(preproc.avg_corners);
                        straight.straightenImage(cam_mat, res, 480, 640);
                        Mat trans_mat = straight.getTransMatrix();

                        finger = find_finger(1, trans_mat, cam_mat, 480,640);

                        imshow("Video", finger);
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
        cvReleaseCapture( &cv_cap );
        cvDestroyWindow("Video");
        waitKey(0);
        return 0;
}
