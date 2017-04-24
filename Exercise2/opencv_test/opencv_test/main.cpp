#include "stdafx.h"
#include <stdio.h>
#include <opencv2\opencv.hpp>
#include "CostVolume.h"

int main()
{
	std::string filepath = "..\\images\\";
	cv::Mat img1 = cv::imread(filepath+"tsukuba_left.png", CV_LOAD_IMAGE_COLOR);
	cv::Mat img2 = cv::imread(filepath+"tsukuba_right.png", CV_LOAD_IMAGE_COLOR);

	//cv::Mat diff;
	//cv::absdiff(img1, img2, diff);

	std::vector<cv::Mat> costVolumeLeft;
	std::vector<cv::Mat> costVolumeRight;

	CostVolume::computeCostVolume(img1, img2, costVolumeLeft, costVolumeRight, 5, 15);

	//cv::imshow("result", diff);
	cv::waitKey(0);
	return 0;
}