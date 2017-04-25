#include "stdafx.h"
#include "CostVolume.h"

using namespace cv;
using namespace std;

void CostVolume::computeCostVolume(const cv::Mat &imgLeft, const cv::Mat &imgRight, std::vector<cv::Mat>
	&costVolumeLeft, std::vector<cv::Mat> &costVolumeRight, int windowSize, int maxDisp) {

	//printf("Width = %d, Height = %d", imgLeft.cols, imgLeft.rows);
	
	printf("Started computing CostVolume...\n");

	int width = imgLeft.cols;
	int height = imgLeft.rows;
	
	int w = windowSize / 2; //.. 5 / 2 = 2;
	
try
{

	//d = Disparity
	for (int d = 0; d < maxDisp; d++) {

		Mat1i costL(height, width);
		Mat1i costR(height, width);

		//For each row of the image
		for (int i = 0; i < width; i++) {
			//For each column of the image
			for (int j = 0; j < height; j++) {
				//Window
				int sum_w = 0;
				for (int k = 0; k < windowSize; k++) {
					for (int l = 0; l < windowSize; l++) {

						//int d_tmp = 5;

						int x1 = i + k - w;
						int x2 = x1 + d;
						//int x2 = d_tmp + i + k - w;
						int y = j + l - w;

						//check if pixel is within image
						if (x1 >= 0 && x1 < width && x2 >= 0 && x2 < width && y >= 0 && y < height) {
							Vec3b color1 = imgLeft.at<Vec3b>(y, x1); // vgl Matlab ;)
							Vec3b color2 = imgRight.at<Vec3b>(y, x2);

							Vec3b diff;
							absdiff(color1, color2, diff);
							sum_w += sum(diff)[0];
							//sum_w += (diff[0] + diff[1] + diff[2]);
						}
					}
				}
				costL.at<int>(j, i) = sum_w;
				if ((i+d) < width) // Otherwise out of range exception
					costR.at<int>(j, i + d) = sum_w;
			}
		}
		costVolumeLeft.at(d) = costL;
		costVolumeRight.at(d) = costR;
		
		printf("Finished for Disparity %d\n", d); // Just for debugging purpose
	}

}
catch ( cv::Exception & e )
{
 cerr << e.msg << endl; // output exception message
}


}













CostVolume::CostVolume()
{
}
CostVolume::~CostVolume()
{
}
