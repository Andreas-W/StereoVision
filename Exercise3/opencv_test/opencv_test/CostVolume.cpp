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

	float gamma_c = 7.0f; //values from paper
	float gamma_p = 36.0f;
	
try
{

	//d = Disparity
	for (int d = 0; d < maxDisp; d++) {

		Mat1f costL(height, width);
		Mat1f costR(height, width);

		//For each row of the image
		for (int i = 0; i < width; i++) {
			//For each column of the image
			for (int j = 0; j < height; j++) {
				//Window
				float sum_w = 0; //sum of weigths
				float sum_v = 0; //sum of values (weight * absdiff)

				
				if (i + d < width) {
					//Center Pixel colors
					Vec3b l_color = imgLeft.at<Vec3b>(j, i + d);
					Vec3b r_color = imgRight.at<Vec3b>(j, i);

					for (int k = 0; k < windowSize; k++) {
						for (int l = 0; l < windowSize; l++) {

							//int d_tmp = 5;

							int x1 = i + k - w;
							int x2 = x1 + d;
							//int x2 = d_tmp + i + k - w;
							int y = j + l - w;

							//check if pixel is within image
							if (x1 >= 0 && x1 < width && x2 >= 0 && x2 < width && y >= 0 && y < height) {
								//Colors at position in window
								Vec3b color1 = imgLeft.at<Vec3b>(y, x2);
								Vec3b color2 = imgRight.at<Vec3b>(y, x1);
								//Absolute difference
								//----
								Vec3b v_diff;
								absdiff(color1, color2, v_diff);
								float diff = sum(v_diff)[0]; //Color difference


								//Weights
								//----
								//Color differences: pixel at window pos to center pixel
								absdiff(l_color, color1, v_diff); //Left image
								float d_cl = sum(v_diff)[0];
								absdiff(r_color, color2, v_diff); //right image
								float d_cr = sum(v_diff)[0];

								//Spatial difference: distance from window pos to center
								float d_p = sqrt(pow((float)(k - w), 2) + pow((float)(l - w), 2));
								//printf("d_p = %.5f\n", d_p);

								float w_l = exp(-((d_cl / gamma_c) + (d_p / gamma_p)));
								float w_r = exp(-((d_cr / gamma_c) + (d_p / gamma_p)));

								//if (i == 55 && j == 126 && k ==1 && l == 2) {
								//	printf("w_l = %.5f, w_r = %.5f\n", w_l, w_r);
								//}

								float weight = w_l * w_r;

								//if (weight <= 0.0f) {
								//	printf("w_l = %.5f, w_r = %.5f\n", w_l, w_r);
								//}

								sum_w += weight;
								sum_v += (weight * diff);


								//int r = abs((int)color1.val[0] - (int)color2.val[0]);
								//int g = abs((int)color1.val[1] - (int)color2.val[1]);
								//int b = abs((int)color1.val[2] - (int)color2.val[2]);
								//sum_w += (r + g + b);
							}
						}
					}
				}
				float cost = 0.0f;
				if (sum_w > 0.0f) {
					cost = (sum_v / sum_w);

					//if (i % 16 == 0 && j % 16 == 0) {
					//	printf("cost = %.5f\n", cost);
					//}
				}
				//printf("cost = %.5f\n", cost);
				costR.at<float>(j, i) = cost;
				if ((i + d) < width) // Otherwise out of range exception
					costL.at<float>(j, i + d) = cost;
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
