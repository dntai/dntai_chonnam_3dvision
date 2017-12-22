#include "CameraCalibrator.h"
#include <iomanip>
#include <string>       // std::string
#include <iostream>     // std::cout
#include <sstream>      // std::stringstream

namespace Vision3D
{
	namespace CamCalib
	{
		// construction
		CameraCalibrator::CameraCalibrator() : flag(0), mustInitUndistort(true)
		{

		};

		// getter
		cv::Mat CameraCalibrator::getCameraMatrix()
		{ 
			return cameraMatrix; 
		}

		cv::Mat CameraCalibrator::getDistCoeffs()
		{ 
			return distCoeffs; 
		}

		// Open chessboard images and extract corner points
		int CameraCalibrator::addChessboardImage(const cv::Mat &image, cv::Size & boardSize, std::vector<cv::Point2f> &imageCorners)
		{
			// the points on the chessboard
			std::vector<cv::Point3f> objectCorners;

			// 3D Scene Points: Initialize the chessboard corners in the chessboard reference frame
			// The corners are at 3D location (X,Y,Z) = (i, j, 0)
			for (int i = 0; i<boardSize.height; i++) 
			{
				for (int j = 0; j<boardSize.width; j++) 
				{
					objectCorners.push_back(cv::Point3f(i, j, 0.0f));
				}
			}

			int successes = findChessboardImage(image, boardSize, imageCorners);
			if (successes > 0)
			{
				addPoints(imageCorners, objectCorners);
			}

			return successes;
		}

		// Find chessboard images and extract corner points
		int CameraCalibrator::findChessboardImage(const cv::Mat &image, cv::Size & boardSize, std::vector<cv::Point2f> &imageCorners)
		{
			// 2D Image points:
			int successes = 0;
			cv::Size size;
			int scale = 1;
			int maxSize = std::max(image.size().width, image.size().height);
			while (maxSize / scale >= 640) scale = scale * 2;
			cv::Mat grayScale, gray;

			cv::cvtColor(image, gray, CV_BGR2GRAY);
			cv::resize(gray, grayScale, cv::Size(image.size().width / scale, image.size().height / scale));
			// cv::GaussianBlur(grayScale, grayScale, cv::Size(5, 5), 0.1);
			cv::threshold(grayScale, grayScale, 0, 255, CV_THRESH_OTSU);

			// cv::imshow("Gray", grayScale);

			// Get the chessboard corners
			bool found = cv::findChessboardCorners(grayScale, boardSize, imageCorners, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FILTER_QUADS | CV_CALIB_CB_FAST_CHECK);
			if (found)
			{
				// Re-scale image corners
				for (int i = 0; i < imageCorners.size(); i++)
				{
					imageCorners[i] = imageCorners[i] * scale;
				}

				// Get subpixel accuracy on the corners
				cv::cornerSubPix(gray, imageCorners, cv::Size(11, 11), cv::Size(-1, -1),
					cv::TermCriteria(cv::TermCriteria::MAX_ITER | cv::TermCriteria::EPS,
						1000,		// max number of iterations 
						0.001)		// min accuracy
				);

				// If we have a good board, add it to our data
				if (imageCorners.size() == boardSize.area())
				{
					successes = 1;
				}
			}

			return successes;
		}

		// Add scene points and corresponding image points
		void CameraCalibrator::addPoints(const std::vector<cv::Point2f>& imageCorners, const std::vector<cv::Point3f>& objectCorners) {

			// 2D image points from one view
			imagePoints.push_back(imageCorners);
			// corresponding 3D scene points
			objectPoints.push_back(objectCorners);
		}

		// Calibrate the camera
		// returns the re-projection error
		double CameraCalibrator::calibrate(cv::Size imageSize)
		{
			// undistorter must be reinitialized
			mustInitUndistort = true;

			//Output rotations and translations
			std::vector<cv::Mat> rvecs, tvecs;

			// start calibration
			return
				calibrateCamera(objectPoints, // the 3D points
					imagePoints,  // the image points
					imageSize,    // image size
					cameraMatrix, // output camera matrix
					distCoeffs,   // output distortion matrix
					rvecs, tvecs, // Rs, Ts 
					flag);        // set options
								  //					,CV_CALIB_USE_INTRINSIC_GUESS);

		}

		// remove distortion in an image (after calibration)
		cv::Mat CameraCalibrator::undistort(const cv::Mat &image) {

			cv::Mat undistorted;

			if (mustInitUndistort) { // called once per calibration

				cv::initUndistortRectifyMap(
					cameraMatrix,  // computed camera matrix
					distCoeffs,    // computed distortion matrix
					cv::Mat(),     // optional rectification (none) 
					cv::Mat(),     // camera matrix to generate undistorted
					cv::Size(image.size().width, image.size().height),// image.size(),  // size of undistorted
					CV_32FC1,      // type of output map
					map1, map2);   // the x and y mapping functions

				mustInitUndistort = false;
			}

			// Apply mapping functions
			cv::remap(image, undistorted, map1, map2,
				cv::INTER_LINEAR); // interpolation type

			return undistorted;
		}

		// Set the calibration options
		// 8radialCoeffEnabled should be true if 8 radial coefficients are required (5 is default)
		// tangentialParamEnabled should be true if tangeantial distortion is present
		void CameraCalibrator::setCalibrationFlag(bool radial8CoeffEnabled, bool tangentialParamEnabled) {

			// Set the flag used in cv::calibrateCamera()
			flag = 0;
			if (!tangentialParamEnabled) flag += CV_CALIB_ZERO_TANGENT_DIST;
			if (radial8CoeffEnabled) flag += CV_CALIB_RATIONAL_MODEL;
		}

		int CameraCalibrator::saveCameraParameters(const std::string path)
		{
			int success = 0;

			try
			{
				// Store everything in a xml file
				cv::FileStorage fs(path.c_str(), cv::FileStorage::WRITE);
				fs << "Intrinsic" << this->cameraMatrix;
				fs << "Distortion" << this->distCoeffs;
				success = 1;
			}
			catch (...)
			{

			}

			return success;
		}
		int CameraCalibrator::readCameraParameters(const std::string path)
		{
			int success = 0;

			try
			{
				cv::FileStorage fs(path.c_str(), cv::FileStorage::READ);
				fs["Intrinsic"] >> this->cameraMatrix;
				fs["Distortion"] >> this->distCoeffs;
				success = 1;
			}
			catch (...)
			{

			}

			return success;
		}

		void CameraCalibrator::showCameraParameters()
		{
			// display camera matrix
			cv::Mat cameraMatrix = this->getCameraMatrix();
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
			cv::Mat distoCoeffs = this->getDistCoeffs();
			std::cout << "Distortion " << distoCoeffs.cols << " coefficients: ";
			for (int i = 0; i < distoCoeffs.cols; i++)
			{
				std::cout << "[" << distoCoeffs.at<double>(0, i) << "]" << " ";
			}
			std::cout << std::endl;
		}

		std::wstring CameraCalibrator::toStringCameraParameters()
		{
			std::wostringstream ss;
			
			// display camera matrix
			cv::Mat cameraMatrix = this->getCameraMatrix();
			ss << " Camera Matrix: " << cameraMatrix.rows << "x" << cameraMatrix.cols << std::endl;
			for (int i = 0; i < 3; i++)
			{
				for (int j = 0;j < 3;j++)
				{
					ss << "[" << std::setw(10) << cameraMatrix.at<double>(i, j);
					ss << "] ";
				}
				ss << std::endl;
			}

			// display distortion coefficients
			cv::Mat distoCoeffs = this->getDistCoeffs();
			ss << "Distortion " << distoCoeffs.cols << " coefficients: ";
			for (int i = 0; i < distoCoeffs.cols; i++)
			{
				ss << std::endl;
				ss << "[" << distoCoeffs.at<double>(0, i) << "]" << " ";
			}
			ss << std::endl;
			return ss.str();
		}
	}
}