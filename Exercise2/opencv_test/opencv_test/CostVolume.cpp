#include "stdafx.h"
#include "CostVolume.h"

using namespace cv;

void CostVolume::computeCostVolume(const cv::Mat &imgLeft, const cv::Mat &imgRight, std::vector<cv::Mat>
	&costVolumeLeft, std::vector<cv::Mat> &costVolumeRight, int windowSize, int maxDisp) {

	//printf("Width = %d, Height = %d", imgLeft.cols, imgLeft.rows);
	
	int width = imgLeft.cols;
	int height = imgLeft.rows;

	int w = windowSize / 2; //.. 5 / 2 = 2;
	
	/*for (int i = w; i < width - w ; i++) {
		for (int j = w; j < height - w; j++) {
			for (int x = 0; x < windowSize; x++) {
				for (int y = 0; y < windowSize; y ++) {

					imgLeft[i+x-w]

				}
			}
		}
	}*/

	//d = Disparity
	for (int d = 0; d <= maxDisp; d++) {

		Mat1i costL(width, height);
		Mat1i costR(width, height);

		//For each row of the image
		for (int i = 0; i < width; i++) {
			//For each column of the image
			for (int j = 0; j < height; j++) {
				//Window
				int sum_w = 0;
				for (int k = 0; k < windowSize; k++) {
					for (int l = 0; l < windowSize; l++) {

						int x1 = i + k - w;
						int x2 = d + i + k - w;
						int y = j + l - w;

						//check if pixel is within image
						if (x1 >= 0 && x1 < width && x2 < width && y >= 0 && y < height) {
							Vec3b color1 = imgLeft.at<Vec3b>(x1, y);
							Vec3b color2 = imgRight.at<Vec3b>(x2, y);

							Vec3b diff;
							absdiff(color1, color2, diff);
							sum_w += sum(diff)[0];

						}
					}
				}
				costL.at<int>(i, j) = sum_w;
				costR.at<int>(i + d, j) = sum_w;
			}
		}
		costVolumeLeft.at(d) = costL;
		costVolumeRight.at(d) = costR;
	}
}













CostVolume::CostVolume()
{
}
CostVolume::~CostVolume()
{
}
