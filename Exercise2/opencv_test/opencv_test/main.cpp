#include "stdafx.h"
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <opencv2\opencv.hpp>
#include "CostVolume.h"

using namespace cv;
using namespace std;

int main()
{
	string filepath = "..\\images\\";
	Mat img1 = imread(filepath+"tsukuba_left.png", CV_LOAD_IMAGE_COLOR);
	Mat img2 = imread(filepath+"tsukuba_right.png", CV_LOAD_IMAGE_COLOR);
	
	imshow("Left", img1);
	imshow("Right", img2);
	
	//cv::Mat diff;
	//cv::absdiff(img1, img2, diff);
	//cv::imshow("result", diff);

	int windowSize = 5; // 5
	int maxDisp = 15; // 15

	// Compute Cost Volume
	vector<Mat> costVolumeLeft(maxDisp); // Need to allocate memory
	vector<Mat> costVolumeRight(maxDisp);
	CostVolume::computeCostVolume(img1, img2, costVolumeLeft, costVolumeRight, windowSize, maxDisp);

	/*
	// Display the cost volume maps
	for (int i = 0; i < maxDisp; i++)
	{
		Mat costVolumeMat1 = costVolumeLeft.at(i);
		Mat costVolumeMat2 = costVolumeRight.at(i);

		// Normalize Values to a grayscaled image to display them
		normalize(costVolumeMat1, costVolumeMat1, 255, 0, NORM_MINMAX);
		normalize(costVolumeMat2, costVolumeMat2, 255, 0, NORM_MINMAX);

		// Set leading '0' to the number
		ostringstream ss;
		ss << setw(3) << std::setfill('0') << i;
		string s_number(ss.str());
		
		// Save images otherwise for some bug they cannot be displayed
		imwrite( filepath+"DisparityMapLeft"  + s_number + ".jpg", costVolumeMat1 );
		imwrite( filepath+"DisparityMapRight" + s_number + ".jpg", costVolumeMat2 );

		// Display 
		Mat disMap1 = imread(filepath+"DisparityMapLeft"  + s_number + ".jpg");
		imshow("Disparity Mat Left" + s_number, disMap1);
		Mat disMap2 = imread(filepath+"DisparityMapRight"  + s_number + ".jpg");
		imshow("Disparity Mat Right" + s_number, disMap2);

	}
	*/

	// Just for testing
	/*for (int i = 0; i < costVolumeMat1.rows; i++)  
	{
		for (int j = 0; j < costVolumeMat1.cols; j++)
		{
			printf("Val: %i\n", costVolumeMat1.at<int>(i,j));
		}
	}*/

	Mat1i disparityL(img1.rows, img1.cols, 0);
	Mat1i disparityR(img1.rows, img1.cols, 0);

	Mat1i minL(img1.rows, img1.cols, -1);
	Mat1i minR(img1.rows, img1.cols, -1);

	for (int d = 0; d < maxDisp; d++) {
		Mat cvL = costVolumeLeft.at(d);
		Mat cvR = costVolumeRight.at(d);
		for (int i = 0; i < img1.rows; i++) {
			for (int j = 0; j < img1.cols; j++) {
				try {
					//Left
					int min = minL.at<int>(i, j);
					int val = cvL.at<int>(i, j);
					if (min == -1 || val < min) {
						minL.at<int>(i, j) = val;
						disparityL.at<int>(i, j) = d;
					}
					//right;
					min = minR.at<int>(i, j);
					val = cvR.at<int>(i, j);
					if (min == -1 || val < min) {
						minR.at<int>(i, j) = val;
						disparityR.at<int>(i, j) = d;
					}
				}
				catch (Exception & e)
				{
					cerr << e.msg << endl; // output exception message
				}
			}
		}
	}

	//disparityL *= (255 / maxDisp);
	//disparityR *= (255 / maxDisp);

	cv::normalize(disparityL, disparityL, 0, 255, NORM_MINMAX);
	cv::normalize(disparityR, disparityR, 0, 255, NORM_MINMAX);


	cv::imwrite(filepath + "_DisparityMapLeft_.jpg", disparityL);
	cv::imwrite(filepath + "_DisparityMapRight_.jpg", disparityR);

	Mat disMap1 = imread(filepath + "_DisparityMapLeft_.jpg");
	imshow("Disparity Mat Left", disMap1);
	Mat disMap2 = imread(filepath + "_DisparityMapRight_.jpg");
	imshow("Disparity Mat Right", disMap2);

	cv::waitKey(0);
	return 0;
}