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

	float gamma_c = 7.0f; //7.0f values from paper
	float gamma_p = 9.0f; // 14.0f; //33.0f;//36.0f;
	
try
{
	/*
	Performance Improvements:
	* Because image.at(y,x) is slow, all image acces is replaced by pointers to image columns
	* Distances in the window are precomputed
	* CIELab color space is now precomputed and stored in a separate mat
	*/

	//precompute spatial distances
	Mat1f distMap(windowSize, windowSize);
	for (int k = 0; k < windowSize; k++) {
		for (int l = 0; l < windowSize; l++) {
			distMap.at<float>(k, l) = sqrt(pow((float)(k - w), 2) + pow((float)(l - w), 2)) / gamma_p;
		}
	}
	
	//Convert to CIELab
	Mat3b imgLeft_LAB(width, height);
	Mat3b imgRight_LAB(width, height);
	cvtColor(imgLeft, imgLeft_LAB, CV_RGB2Lab);
	cvtColor(imgRight, imgRight_LAB, CV_RGB2Lab);

	
	//d = Disparity
	for (int d = 0; d < maxDisp; d++) {

		Mat1f costL(height, width);
		Mat1f costR(height, width);

		//For each row of the image
		for (int i = 0; i < height; i++) {
			
			// RGB
			const Vec3b* l_pixel = imgLeft.ptr<Vec3b>(i); // point to first pixel in row
			const Vec3b* r_pixel = imgRight.ptr<Vec3b>(i);

			// Lab
			Vec3b* l_pixel_LAB = imgLeft_LAB.ptr<Vec3b>(i);
			Vec3b* r_pixel_LAB = imgRight_LAB.ptr<Vec3b>(i);
			
			//For each column of the image
			for (int j = 0; j < width; j++) {
				//Window
				float sum_v = 0; //sum of values (weights * absdiff)
				float sum_w = 0; //sum of weigths
				
				if (i + d < width) {
					// RGB
					Vec3b l_color = l_pixel[j + d];
					Vec3b r_color = r_pixel[j];
				
					// Lab
					Vec3b l_color_LAB = l_pixel_LAB[j + d];
					Vec3b r_color_LAB = r_pixel_LAB[j];

					//for each window row
					for (int k = 0; k < windowSize; k++) {
					
						int y = i + k - w;
						if (y >= 0 && y < height) {

							// RGB
							const Vec3b* l_w_pixel = imgLeft.ptr<Vec3b>(i + k - w); // point to first pixel in row
							const Vec3b* r_w_pixel = imgRight.ptr<Vec3b>(i + k - w);

							//Lab
							Vec3b* l_w_pixel_LAB = imgLeft_LAB.ptr<Vec3b>(i + k - w);
							Vec3b* r_w_pixel_LAB = imgRight_LAB.ptr<Vec3b>(i + k - w);

							//for each window col
							for (int l = 0; l < windowSize; l++) {
						
								int x1 = j + l - w;
								int x2 = x1 + d;
						
								//check if pixel is within image
								if (x1 >= 0 && x1 < width && x2 >= 0 && x2 < width) {
									/* --------------------------- */
									Vec3b col1 = l_w_pixel_LAB[x2];
									Vec3b col2 = r_w_pixel_LAB[x1];

									// computing color difference for left and right image - paper eq. 5
									float d_cl = sqrt(col1[0] * l_color_LAB[0] + col1[1] * l_color_LAB[1] + col1[2] * l_color_LAB[2]);
									float d_cr = sqrt(col2[0] * r_color_LAB[0] + col2[1] * r_color_LAB[1] + col2[2] * r_color_LAB[2]);
							
									// d_p / gamma_p is now precomputed
									float d_p = distMap.at<float>(k, l);

									// strength of grouping by color similarity and strength of grouping by proximity
									float w_l = exp(-((d_cl / gamma_c) + d_p)); // paper eq. 9
									float w_r = exp(-((d_cr / gamma_c) + d_p));
							
									/* --------------------------- */
									//Colors at position in window
									Vec3b color1 = l_w_pixel[x2];
									Vec3b color2 = r_w_pixel[x1];
							
									//Absolute difference
									Vec3b v_diff;
									absdiff(color1, color2, v_diff);
									// pixel-based raw matching cost computed by using the colors of q and ¯qd. When using AD (absolute difference)
									float e_0 = sum(v_diff)[0]; //Color difference - Eq. 11

									// The dissimilarity between pixel p in left image and p' in right image - Eq. 10
									float weight = w_l * w_r;
							
									sum_v += (weight * e_0); // numerator
									sum_w += weight;		  // denominator
							
									/* --------------------------- */
								} // end if check pixel is in window
							} // end l
						} // if (i + k - w) < height
					} // end k
				} // end if i + d < width

				float cost = 0.0f;
				if (sum_w > 0.0f) {
					cost = (sum_v / sum_w); // Eq. 10 - The dissimilarity between pixel p and ¯pd,
				}
				//printf("cost = %.5f\n", cost);
				costR.at<float>(i, j) = cost;
				if ((j + d) < width) // Otherwise out of range exception
					costL.at<float>(i, j + d) = cost;


			} // end j
		} // end i
		costVolumeLeft.at(d) = costL;
		costVolumeRight.at(d) = costR;
		
		printf("Finished for Disparity %d\n", d);
	} // end d


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
