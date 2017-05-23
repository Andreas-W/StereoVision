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
	//IplImage* img = cvLoadImage("..\\images\\tsukuba_left.png");
	//Mat img1 = Mat(img);
	//img = cvLoadImage("..\\images\\tsukuba_right.png");
	//Mat img2 = Mat(img);
	Mat img1 = imread(filepath+"tsukuba_left.png", CV_LOAD_IMAGE_COLOR);
	Mat img2 = imread(filepath+"tsukuba_right.png", CV_LOAD_IMAGE_COLOR);
	
	//imshow("Left", img1);
	//imshow("Right", img2);

	int windowSize = 10; // 5
	int maxDisp = 15; // 15

	// Compute Cost Volume
	vector<Mat> costVolumeLeft(maxDisp); // Need to allocate memory
	vector<Mat> costVolumeRight(maxDisp);
	CostVolume::computeCostVolume(img1, img2, costVolumeLeft, costVolumeRight, windowSize, maxDisp);

	Mat1i disparityL(img1.rows, img1.cols, 0);
	Mat1i disparityR(img1.rows, img1.cols, 0);

	Mat1f minL(img1.rows, img1.cols, -1);
	Mat1f minR(img1.rows, img1.cols, -1);

	for (int d = 0; d < maxDisp; d++) {
		Mat cvL = costVolumeLeft.at(d);
		Mat cvR = costVolumeRight.at(d);
		for (int i = 0; i < img1.rows; i++) {
			for (int j = 0; j < img1.cols; j++) {
				try {
					//Left
					float min = minL.at<float>(i, j);
					float val = cvL.at<float>(i, j);
					if (min == -1 || val < min) {
						minL.at<float>(i, j) = val;
						disparityL.at<int>(i, j) = d;
					}
					//right;
					min = minR.at<float>(i, j);
					val = cvR.at<float>(i, j);
					if (min == -1 || val < min) {
						minR.at<float>(i, j) = val;
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

	disparityL *= (255 / maxDisp); //Should be same depth-colors as groundTruth
	//disparityR *= (255 / maxDisp);

	//cv::normalize(disparityL, disparityL, 0, 255, NORM_MINMAX);
	//cv::normalize(disparityR, disparityR, 0, 255, NORM_MINMAX);

	//imshow("Left Disparity Map", disparityL);
	std::string fnameL = filepath + "DisparityMapLeft_w" + to_string(windowSize) + "_d" + to_string(maxDisp) + ".png";
	//std::string fnameR = filepath + "DisparityMapRight_w" + to_string(windowSize) + "_d" + to_string(maxDisp) + ".png";
	cv::imwrite(fnameL, disparityL);
	//cv::imwrite(fnameR, disparityR);

	Mat disMap1 = imread(fnameL);
	imshow("Disparity Mat Left", disMap1);
	//Mat disMap2 = imread(fnameR);
	//imshow("Disparity Mat Right", disMap2);


	/*
	//--------------------------
	//Evaluation
	//---------------
	Mat img_gt = imread(filepath + "tsukuba_gt.png", CV_LOAD_IMAGE_GRAYSCALE);
	//Crop images to ignore borders
	Rect R(Point(18,18), Point(366, 270)); //Create a rect 
	Mat1b m_gt = img_gt(R);
	Mat1b m_dL = disparityL(R);
	//Mat1b m_dR = disparityR(R);

	Mat1b m_eL = abs(m_gt - m_dL);
	//Mat1b m_eR = abs(m_gt - m_dR);

	threshold(m_eL, m_eL, (255 / maxDisp), 255, THRESH_BINARY);
	//threshold(m_eR, m_eR, (255 / maxDisp), 255, THRESH_BINARY);

	fnameL = filepath + "ErrorMapLeft_w" + to_string(windowSize) + "_d" + to_string(maxDisp) + ".png";
	//fnameR = filepath + "ErrorMapRight_w" + to_string(windowSize) + "_d" + to_string(maxDisp) + ".png";
	cv::imwrite(fnameL, m_eL);
	//cv::imwrite(fnameR, m_eR);

	disMap1 = imread(fnameL);
	imshow("Error Mat Left", disMap1);
	//disMap2 = imread(fnameR);
	//imshow("Error Mat Right", disMap2);

	*/

	cv::waitKey(0);
	return 0;
}