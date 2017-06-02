#pragma once

//#include <stdafx.h>
#include <stdio.h>
#include <opencv2\opencv.hpp>
class CostVolume
{
public:
	CostVolume();
	~CostVolume();

	static void computeCostVolume(const cv::Mat &imgLeft, const cv::Mat &imgRight, std::vector<cv::Mat>
		&costVolumeLeft, std::vector<cv::Mat> &costVolumeRight, int windowSize, int maxDisp);

	static cv::Vec3b colorAt(const cv::Mat &img, int y, int x);
};

