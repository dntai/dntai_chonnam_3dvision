#pragma once

#include <vector>
#include <iostream>

#include <opencv2/core/core.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include <opencv2/highgui/highgui.hpp>

namespace Vision3D
{
	namespace CamCalib
	{
		class CameraCalibrator
		{
		private:
			// input points
			std::vector<std::vector<cv::Point3f> > objectPoints;
			std::vector<std::vector<cv::Point2f> > imagePoints;
			// output matrices
			cv::Mat cameraMatrix;
			cv::Mat distCoeffs;
			// flag to specify how calibration is done
			int flag;
			// used in image undistortion 
			cv::Mat map1, map2;
			bool mustInitUndistort;
		public:
			CameraCalibrator();
			
			// Open the chessboard images and extract corner points
			int addChessboardImage(const cv::Mat &image, cv::Size & boardSize, std::vector<cv::Point2f> &imageCorners);

			// Find chessboard images and extract corner points
			int findChessboardImage(const cv::Mat &image, cv::Size & boardSize, std::vector<cv::Point2f> &imageCorners);
			
			// Add scene points and corresponding image points
			void addPoints(const std::vector<cv::Point2f>& imageCorners, const std::vector<cv::Point3f>& objectCorners);
			
			// Calibrate the camera
			double calibrate(cv::Size imageSize);
			
			// Set the calibration flag
			void setCalibrationFlag(bool radial8CoeffEnabled = false, bool tangentialParamEnabled = false);
			
			// Remove distortion in an image (after calibration)
			cv::Mat undistort(const cv::Mat &image);
			
			// Getters
			cv::Mat getCameraMatrix();
			cv::Mat getDistCoeffs();

			// Save & Read Camera Parameters
			int saveCameraParameters(const std::string path);
			int readCameraParameters(const std::string path);
			void showCameraParameters();
			std::wstring toStringCameraParameters();
		};
	}
}
