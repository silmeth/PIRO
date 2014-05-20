#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/contrib/contrib.hpp"

#include <iostream>
#include <stdio.h>

using namespace std;
using namespace cv;

/*Tu kod kalibracji z tego projektu z szachownicą. Wszystko się rozbija o
funkcję calibrateCamera (szokujące, nie? :P). Są dwie funkcje z różnymi
argumentami, obie praktycznie jednakowe, obie wklejam poniżej.

Po kalibracji używają uzyskanych współczynników w ten sposób:
Najpierw zapisują do macierzy:
*/

Mat d_macierzKamery(3,3,6);
for(int i=0; i<3; i++){
	for(int j=0; j<3; j++){
		d_macierzKamery.at<double>(i,j)=intrinsic[i][j];
	}
}
Mat d_wspolczynnikiZnieksztalcen(1,8,6);
for(int i=0; i<8; i++){
	d_wspolczynnikiZnieksztalcen.at<double>(0,i)=distCoeffs[0][i];
}

    /*
Potem używają tego do kalibracji odczytanej klatki:
*/
OK_Chessboard = findChessboardCorners(zdjecie, d_rozmiarSzachownicy, t_punktyObiektu2D, CALIB_CB_FAST_CHECK);
if (OK_Chessboard) {
	if(zdjecie.type() == 16){
		cvtColor(zdjecie, zdjecie, CV_BGR2GRAY);
	}
    cornerSubPix(zdjecie, t_punktyObiektu2D,
    Size(t_wymiarDoSub,t_wymiarDoSub), Size(-1,-1),
    TermCriteria(cv::TermCriteria::MAX_ITER + TermCriteria::EPS,30,0.05));

    solvePnP(d_punktyObiektu3D, t_punktyObiektu2D, d_macierzKamery, d_wspolczynnikiZnieksztalcen,
             d_orientacjaKamery, d_pozycjaKamery);
    Mat t_orientacja;
}

bool ChessBoard::calibre_CAM(unsigned int numCornersHor, unsigned intnumCornersVer,
							 unsigned int numBoards, Mat * pic, unsigned intSquare_Size) {
    bool STATE = true;
    //int numSquares = (numCornersHor-1) * (numCornersVer-1);
    Size board_sz ;//= Size(numCornersHor-1, numCornersVer-1);
    board_sz.width = numCornersHor-1;
    board_sz.height = numCornersVer-1;

    vector<vector<Point2f> > image_points;
    vector<Point2f> corners;
    vector<vector<Point3f> > object_points;

    vector<Point3f> obj;
    for (int i=0; i<board_sz.height; i++) {
        for (int j=0; j<board_sz.width; j++) {
            obj.push_back(cv::Point3f(i*Square_Size, j*Square_Size, 0.0f));
        }
    }

    Mat image;
    Mat gray_image;

    unsigned int successes = 0;
    bool found = false;

    while(successes<numBoards) {
        image = pic[successes].clone();//imread(source[successes],1);
        successes ++;
        cvtColor(image, gray_image, CV_BGR2GRAY);
        found = false;
        //found = findChessboardCorners(image, board_sz, corners,CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FILTER_QUADS);
        found = findChessboardCorners(image, board_sz, corners,CALIB_CB_FAST_CHECK);

        //drawChessboardCorners(image,board_sz,corners,found);
        //imshow("cas",image);
        //cvWaitKey(0);

        cout<<"Processing"<<successes<<" status: "<<found<<endl;
        if(found) {
            cornerSubPix(gray_image, corners, Size(11, 11), Size(-1,-1),
            		TermCriteria(TermCriteria::MAX_ITER +TermCriteria::EPS,30,0.05));
            image_points.push_back(corners);
            object_points.push_back(obj);
        }
        else {
            STATE = false;
        }
    }

    Mat intrinsic = Mat(3, 3, CV_32FC1);
    Mat distCoeffs;
    vector<Mat> rvecs;
    vector<Mat> tvecs;

    intrinsic.ptr<float>(0)[0] = 1;
    intrinsic.ptr<float>(1)[1] = 1;

    calibrateCamera(object_points,image_points, image.size(),
    				intrinsic,distCoeffs, rvecs, tvecs,CV_CALIB_RATIONAL_MODEL);

    FileStorage fs("CALIB.txt", FileStorage::WRITE);//zapis
    fs<<"intrinsic"<<intrinsic;
    fs<<"distCoeffs"<<distCoeffs;
    fs.release();

    return STATE;
}


bool ChessBoard::calibre_CAM(unsigned int numCornersHor, unsigned int
numCornersVer, unsigned int numBoards,string * source, unsigned int
Square_Size ) {
    bool STATE = true;
    //int numSquares = (numCornersHor-1) * (numCornersVer-1);
    Size board_sz ;//= Size(numCornersHor-1, numCornersVer-1);
    board_sz.width = numCornersHor-1;
    board_sz.height = numCornersVer-1;

    vector<vector<Point2f> > image_points;
    vector<Point2f> corners;
    vector<vector<Point3f> > object_points;

    vector<Point3f> obj;
    for (int i=0; i<board_sz.height; i++) {
        for (int j=0; j<board_sz.width; j++) {
            obj.push_back(cv::Point3f(i*Square_Size, j*Square_Size, 0.0f));
        }
    }


    Mat image;
    Mat gray_image;

    unsigned int successes = 0;
    bool found = false;

    while(successes<numBoards) {
        image = imread(source[successes],1);
        successes ++;
        cvtColor(image, gray_image, CV_BGR2GRAY);
        found = false;
        found = findChessboardCorners(image, board_sz, corners,CALIB_CB_FAST_CHECK);
        cout<<"Processing"<<successes<<" status: "<<found<<endl;
        if(found) {
            cornerSubPix(gray_image, corners, Size(11, 11), Size(-1, -1),
            			 TermCriteria(TermCriteria::MAX_ITER +TermCriteria::EPS,30,0.05));

        } else {
            STATE = false;
        }
        image_points.push_back(corners);
        object_points.push_back(obj);

    }

    Mat intrinsic = Mat(3, 3, CV_32FC1);
    Mat distCoeffs;
    vector<Mat> rvecs;
    vector<Mat> tvecs;

    intrinsic.ptr<float>(0)[0] = 1;
    intrinsic.ptr<float>(1)[1] = 1;

    calibrateCamera(object_points,image_points, image.size(), intrinsic,
    				distCoeffs, rvecs, tvecs,CV_CALIB_RATIONAL_MODEL);

    FileStorage fs("CALIB.txt", FileStorage::WRITE);//zapis
    fs<<"intrinsic"<<intrinsic;
    fs<<"distCoeffs"<<distCoeffs;
    fs.release();

    return STATE;
}
