/*------------------------------------------------------------------------------------------*\
   This file contains material supporting chapter 9 of the cookbook:  
   Computer Vision Programming using the OpenCV Library. 
   by Robert Laganiere, Packt Publishing, 2011.

   This program is free software; permission is hereby granted to use, copy, modify, 
   and distribute this source code, or portions thereof, for any purpose, without fee, 
   subject to the restriction that the copyright notice may not be removed 
   or altered from any source or altered source distribution. 
   The software is released on an as-is basis and without any warranties of any kind. 
   In particular, the software is not guaranteed to be fault-tolerant or free from failure. 
   The author disclaims all warranties with regard to this software, any use, 
   and any consequent failure, is purely the responsibility of the user.
 
   Copyright (C) 2010-2011 Robert Laganiere, www.laganiere.name
\*------------------------------------------------------------------------------------------*/

#include <iostream>
#include <iomanip>
#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>

#include "CameraCalibrator.h"

int main()
{

	cv::namedWindow("Image");
	cv::Mat image;
	std::vector<std::string> filelist;

	// generate list of chessboard image filename
	for (int i=1; i<=20; i++) {

		std::stringstream str;
		str << "data/chessboards/chessboard" << std::setw(2) << std::setfill('0') << i << ".jpg";
		std::cout << str.str() << std::endl;

		filelist.push_back(str.str());
		image= cv::imread(str.str(),0);
		cv::imshow("Image",image);
	
		 cv::waitKey(100);
	}

	// Create calibrator object
    CameraCalibrator cameraCalibrator;
	// add the corners from the chessboard
	cv::Size boardSize(6,4);
	cameraCalibrator.addChessboardPoints(
		filelist,	// filenames of chessboard image
		boardSize);	// size of chessboard
		// calibrate the camera
    //	cameraCalibrator.setCalibrationFlag(true,true);
	cameraCalibrator.calibrate(image.size());

    // Image Undistortion
    image = cv::imread(filelist[6]);
	cv::Mat uImage= cameraCalibrator.undistort(image);

	// display camera matrix
	cv::Mat cameraMatrix= cameraCalibrator.getCameraMatrix();
	std::cout << " Camera intrinsic: " << cameraMatrix.rows << "x" << cameraMatrix.cols << std::endl;
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0;j < 3;j++)
		{
			std::cout << "[" << std::setw(10) << cameraMatrix.at<double>(i, j);
			std::cout << "]";
		}
		std::cout << std::endl;
	}
	
	// display distortion coefficients
	cv::Mat distoCoeffs = cameraCalibrator.getDistCoeffs();
	std::cout << "Distortion "<< distoCoeffs.cols <<" coefficients: ";
	for (int i = 0; i < distoCoeffs.cols; i++)
	{
		std::cout << "["<< distoCoeffs.at<double>(0, i) <<"]" << " ";
	}
	std::cout << std::endl;

    imshow("Original Image", image);
    imshow("Undistorted Image", uImage);

	cv::waitKey();
	return 0;
}
