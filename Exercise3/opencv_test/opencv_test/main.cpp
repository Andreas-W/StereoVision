#include "stdafx.h"
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <limits>
#include <opencv2\opencv.hpp>
#include "CostVolume.h"

using namespace cv;
using namespace std;

void evaluation(cv::Mat &disparityL, cv::Mat &disparityR, int maxDisp, int windowSize);
void setClosestDisparityValue(cv::Mat &newDisp, cv::Mat &oldDisp, int i, int j, int d, int &id_close_left, int id_close_right);
void saveAndShowDisparityMaps(cv::Mat disparityL, cv::Mat disparityR, int maxDisp, int windowSize, std::string name);
void refineDisparity(cv::Mat &dispLeft, cv::Mat &dispRight, int scaleDispFactor);

int main()
{
	string filepath = "..\\images\\";

	int windowSize = 11; // 5
	int maxDisp = 15; // 15


	Mat img1 = imread(filepath+"tsukuba_left.png", CV_LOAD_IMAGE_COLOR);
	Mat img2 = imread(filepath+"tsukuba_right.png", CV_LOAD_IMAGE_COLOR);
	

	// Compute Cost Volume
	vector<Mat> costVolumeLeft(maxDisp); // Need to allocate memory
	vector<Mat> costVolumeRight(maxDisp);
	CostVolume::computeCostVolume(img1, img2, costVolumeLeft, costVolumeRight, windowSize, maxDisp);

	// Winner Takes it All - Get final Disparity Maps
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
						disparityL.at<int>(i, j) = d; // Eq. 12 - Winner-Takes-All
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
	
	// imshow and save
	saveAndShowDisparityMaps(disparityL, disparityR, maxDisp, windowSize, "DisparityMapWithoutRefine");
		
	// Task 2 - Refine Disparity Map
	int scaleDispFactor = (255 / maxDisp); // is not used!
	refineDisparity(disparityL, disparityR, scaleDispFactor);
	
	// evaluation
	evaluation(disparityL, disparityR, maxDisp, windowSize);

	// imshow and save
	saveAndShowDisparityMaps(disparityL, disparityR, maxDisp, windowSize, "Refined");

	cv::waitKey(0);
	return 0;
}

void evaluation(cv::Mat &disparityL, cv::Mat &disparityR, int maxDisp, int windowSize) {
	
	string filepath = "..\\images\\";

	Mat img_gt = imread(filepath + "tsukuba_gt.png", CV_LOAD_IMAGE_GRAYSCALE);
	//Crop images to ignore borders
	Rect R(Point(18,18), Point(366, 270)); //Create a rect 
	Mat1b m_gt = img_gt(R);
	Mat1b m_dL = disparityL(R).clone();
	Mat1b m_dR = disparityR(R).clone();

	m_dL = m_dL * (255/maxDisp);
	m_dR = m_dR * (255/maxDisp);

	Mat1b m_eL = abs(m_gt - m_dL);
	Mat1b m_eR = abs(m_gt - m_dR);

	threshold(m_eL, m_eL, (255 / maxDisp), 255, THRESH_BINARY);
	threshold(m_eR, m_eR, (255 / maxDisp), 255, THRESH_BINARY);

	// Get sum of all incorrect pixels
	int sum_left  = (cv::sum( m_eL )[0]) / 255;
	int sum_right = (cv::sum( m_eR )[0]) / 255;
	printf("Sum of incorret Pixels in Left  Image: %i\n",sum_left);
	printf("Sum of incorret Pixels in Right Image: %i\n",sum_right);

	std::string fnameL = filepath + "ErrorMapLeft_w" + to_string(windowSize) + "_d" + to_string(maxDisp) + ".png";
	std::string fnameR = filepath + "ErrorMapRight_w" + to_string(windowSize) + "_d" + to_string(maxDisp) + ".png";
	cv::imwrite(fnameL, m_eL);
	cv::imwrite(fnameR, m_eR);
	
	Mat disMap1 = imread(fnameL);
	cv::imshow("Error Mat Left", disMap1);
	Mat disMap2 = imread(fnameR);
	cv::imshow("Error Mat Right", disMap2);
}

void refineDisparity(cv::Mat &dispLeft, cv::Mat &dispRight, int scaleDispFactor) {
		
	int width = dispLeft.cols;
	int height = dispLeft.rows;
	
	Mat1i markedDispL(height, width, -1);
	Mat1i markedDispR(height, width, -1);

	for (int i = 0; i < height; i++) {
		const int* r_pixel = dispRight.ptr<int>(i); // point to first pixel in row
		const int* l_pixel = dispLeft.ptr<int>(i);

		for (int j = 0; j < width; j++) {
			try {
				int d_r = r_pixel[j];

				if ((j + d_r) < width)
				{
					int d_l = l_pixel[j + d_r];

					//printf("r_color: %d\n",d_r);
					//printf("l_color: %d\n",d_l);

					// Check for inconsitent pixels by comparing each pixel from one image 
					//		with its corresponding pixel in the other image.
					// --> Mark those pixels as invalid whose corresponding disparity is 
					//		different by a value larger than one pixel
					if (abs(d_r - d_l) > 1) {
						// Showing the error as blue - debbuging
						//dispRight.at<Vec3b>(i,j) = Vec3b(15,0,0);
						//dispLeft.at<Vec3b>(i,j + d_r) = Vec3b(15,0,0);

						// This errors happen mostly in occluded or mismatched regions
						markedDispR.at<int>(i,j)       = -1;
						markedDispL.at<int>(i,j + d_r) = -1;

					} else {
						markedDispR.at<int>(i,j)       = d_r;
						markedDispL.at<int>(i,j + d_r) = d_l;
					}
				}

			}
			catch (Exception & e)
			{
				cerr << e.msg << endl; // output exception message
			}
		}
	}
	//saveAndShowDisparityMaps(dispLeft, dispRight, 15, 5, "Error");

	
	// Fill those invalid pixels with the minimum of their closest 
	//		valid left or right neighbor’s disparity
	Mat1i refinedDispL(height, width, -1);
	Mat1i refinedDispR(height, width, -1);

	for (int i = 0; i < height; i++) {
		const int* r_pixel = markedDispR.ptr<int>(i); // point to first pixel in row
		const int* l_pixel = markedDispL.ptr<int>(i);

		 // Save the position of pixel with valid disparity on the left of upcoming pixels
		int r_id_close_left = -width;
		int r_id_close_right = std::numeric_limits<int>::max();
		int l_id_close_left = -width;
		int l_id_close_right = std::numeric_limits<int>::max();

		for (int j = 0; j < width; j++) {
			int d_r = r_pixel[j];
			int d_l = l_pixel[j];
			try {
				/* --------------------------- */
				// Refine Disparity right image
				setClosestDisparityValue(refinedDispR, markedDispR, i, j, d_r, r_id_close_left, r_id_close_right);
			
				/* --------------------------- */
				// Refine Disparity left image
				setClosestDisparityValue(refinedDispL, markedDispL, i, j, d_l, l_id_close_left, l_id_close_right);
			}
			catch (Exception & e)
			{
				cerr << e.msg << endl; // output exception message
			}
		}
	}

	// overwrite them so that they can be evaluated
	dispLeft  = refinedDispL;
	dispRight = refinedDispR;
}

/*
* newDisp ... refinedMat
* oldDisp ... marked with invalid pixels as -1
* i,j     ... poistion
* d       ... disparity value
* id_close_left  ... holds the last valid disparity value left from j (and is updated)
* id_close_right ... Integer max value
*/
void setClosestDisparityValue(cv::Mat &newDisp, cv::Mat &oldDisp, int i, int j, int d, int &id_close_left, int id_close_right) {
	if (d == -1) { 
		// neigbor on the left already saved
		int id_left = id_close_left;

		// look on the neighbors to the right
		int id_right = id_close_right;
		for (int j_r = j+1; j_r < oldDisp.cols; j_r++) { // go if necessary the whole width
			if (oldDisp.at<int>(i,j_r) != -1) {
				id_right = j_r;
				break;
			}
		}

		if (abs(j - id_left) <= abs(j - id_right)) {
			// Take left neighbors disparity
			newDisp.at<int>(i,j) = oldDisp.at<int>(i,id_left);
		}
		if (abs(j - id_left) >  abs(j - id_right)) {
			// Take right neighbors disparity
			newDisp.at<int>(i,j) = oldDisp.at<int>(i,id_right);
		}
	} else {
		id_close_left = j; // Save the position of pixel with valid disparity on the left of upcoming pixels
		newDisp.at<int>(i,j) = oldDisp.at<int>(i,j);
	}
}

// Only for debugging reasons
void saveAndShowDisparityMaps(cv::Mat disparityL, cv::Mat disparityR, int maxDisp, int windowSize, std::string name) {
	
	string filepath = "..\\images\\";
	
	// Compute Pixel Values to visualize again

	Mat1i dispL = disparityL.clone();
	Mat1i dispR = disparityR.clone();

	dispL *= (255 / maxDisp); //Should be same depth-colors as groundTruth
	dispR *= (255 / maxDisp);

	std::string fnameL = filepath + name + "Left_w" + to_string(windowSize) + "_d" + to_string(maxDisp) + ".png";
	std::string fnameR = filepath + name + "Right_w" + to_string(windowSize) + "_d" + to_string(maxDisp) + ".png";
	cv::imwrite(fnameL, dispL);
	cv::imwrite(fnameR, dispR);
	
	Mat disMap1 = imread(fnameL);
	imshow(name + "Disparity Mat Left", disMap1);
	Mat disMap2 = imread(fnameR);
	imshow(name + "Disparity Mat Right", disMap2);
}