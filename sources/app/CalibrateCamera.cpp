#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include <opencv2/calib3d.hpp>
#include <chrono>
#include <thread>
#include <iomanip>

#include <camcalib\CameraCalibrator.h>
using namespace Vision3D::CamCalib;

using namespace cv;

int main(int argc, char *argv[])
{
	Mat image;
	char name[255];

	int success;
	CameraCalibrator cameraCalibrator;
	std::vector<cv::Point2f> imageCorners;

	for (int i = 1; i <= 10; i++)
	{
		sprintf(name, "DATA\\boards1\\board%d.png", i);
		image = cv::imread(name);
		
		success = cameraCalibrator.addChessboardImage(image, cv::Size(6, 4), imageCorners);
		cv::drawChessboardCorners(image, cv::Size(6, 4), imageCorners, success > 0);
		cv::imshow("Chessboard", image);
		cv::waitKey(100);
	}

	// calibrate the camera
	cameraCalibrator.setCalibrationFlag(true,true);
	cameraCalibrator.calibrate(image.size());

	cameraCalibrator.showCameraParameters();

	// Image Undistortion
	sprintf(name, "DATA\\boards\\board%d.png", 6);
	image = cv::imread(name);
	cv::Mat uImage = cameraCalibrator.undistort(image);

	imshow("Original Image", image);
	imshow("Undistorted Image", uImage);

	// Store everything in a xml file
	cameraCalibrator.saveCameraParameters("DATA//calib.xml");

	cv::waitKey(0);

	return EXIT_SUCCESS;
}