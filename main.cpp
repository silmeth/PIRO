#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/contrib/contrib.hpp"

#include "tests.h"
#include "shape_finder.h"
#include "preprocessing.h"
#include "straightener.h"

using namespace std;

int main(int argc, const char** argv) {
	Preprocessing preproc;
	Straightener straight;
	int c;
	IplImage* color_img;
	CvCapture* cv_cap = cvCaptureFromCAM(1);
	cvNamedWindow("Video", 0); // create window
	for(;;) {
		color_img = cvQueryFrame(cv_cap); // get frame
		if(color_img != 0) {
			Mat cam_mat(color_img);
			Mat res;
			preproc.getPage(cam_mat);
			straight.setCorners(preproc.avg_corners);
			straight.straightenImage(cam_mat, res, 300, 423);
			imshow("Video", res);
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
