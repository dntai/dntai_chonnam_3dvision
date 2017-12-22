#include <opencv2/highgui.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <chrono>
#include <thread>

using namespace std;
using namespace cv;

namespace {
	const char* about = "Basic marker detection";
	const char* keys =
		"{d        | 9     | dictionary: DICT_4X4_50=0, DICT_4X4_100=1, DICT_4X4_250=2,"
		"DICT_4X4_1000=3, DICT_5X5_50=4, DICT_5X5_100=5, DICT_5X5_250=6, DICT_5X5_1000=7, "
		"DICT_6X6_50=8, DICT_6X6_100=9, DICT_6X6_250=10, DICT_6X6_1000=11, DICT_7X7_50=12,"
		"DICT_7X7_100=13, DICT_7X7_250=14, DICT_7X7_1000=15, DICT_ARUCO_ORIGINAL = 16}"
		"{v        |       | Input from video file, if ommited, input comes from camera }"
		"{ci       | 0     | Camera id if input doesnt come from video (-v) }"
		"{c        |Data\\aruco_calib.xml | Camera intrinsic parameters. Needed for camera pose }"
		"{l        | 0.1   | Marker side lenght (in meters). Needed for correct scale in camera pose }"
		"{dp       |       | File of marker detector parameters }"
		"{r        |       | show rejected candidates too }"
		"{m        |video  | integration to video }"
		"{p        |Data\\demo.mp4  | integration to video }";
}

/**
*/
static bool readCameraParameters(string filename, Mat &camMatrix, Mat &distCoeffs) {
	FileStorage fs(filename, FileStorage::READ);
	if (!fs.isOpened())
		return false;
	fs["Intrinsic"] >> camMatrix;
	fs["Distortion"] >> distCoeffs;
	return true;
}



/**
*/
static bool readDetectorParameters(string filename, Ptr<aruco::DetectorParameters> &params) {
	FileStorage fs(filename, FileStorage::READ);
	if (!fs.isOpened())
		return false;
	fs["adaptiveThreshWinSizeMin"] >> params->adaptiveThreshWinSizeMin;
	fs["adaptiveThreshWinSizeMax"] >> params->adaptiveThreshWinSizeMax;
	fs["adaptiveThreshWinSizeStep"] >> params->adaptiveThreshWinSizeStep;
	fs["adaptiveThreshConstant"] >> params->adaptiveThreshConstant;
	fs["minMarkerPerimeterRate"] >> params->minMarkerPerimeterRate;
	fs["maxMarkerPerimeterRate"] >> params->maxMarkerPerimeterRate;
	fs["polygonalApproxAccuracyRate"] >> params->polygonalApproxAccuracyRate;
	fs["minCornerDistanceRate"] >> params->minCornerDistanceRate;
	fs["minDistanceToBorder"] >> params->minDistanceToBorder;
	fs["minMarkerDistanceRate"] >> params->minMarkerDistanceRate;
	fs["cornerRefinementMethod"] >> params->cornerRefinementMethod;
	fs["cornerRefinementWinSize"] >> params->cornerRefinementWinSize;
	fs["cornerRefinementMaxIterations"] >> params->cornerRefinementMaxIterations;
	fs["cornerRefinementMinAccuracy"] >> params->cornerRefinementMinAccuracy;
	fs["markerBorderBits"] >> params->markerBorderBits;
	fs["perspectiveRemovePixelPerCell"] >> params->perspectiveRemovePixelPerCell;
	fs["perspectiveRemoveIgnoredMarginPerCell"] >> params->perspectiveRemoveIgnoredMarginPerCell;
	fs["maxErroneousBitsInBorderRate"] >> params->maxErroneousBitsInBorderRate;
	fs["minOtsuStdDev"] >> params->minOtsuStdDev;
	fs["errorCorrectionRate"] >> params->errorCorrectionRate;
	return true;
}

int main(int argc, char *argv[]) 
{
	CommandLineParser parser(argc, argv, keys);
	parser.about(about);

	int dictionaryId = parser.get<int>("d");
	bool showRejected = parser.has("r");
	bool estimatePose = parser.has("c");
	float markerLength = parser.get<float>("l");

	Ptr<aruco::DetectorParameters> detectorParams = aruco::DetectorParameters::create();
	if (parser.has("dp")) 
	{
		bool readOk = readDetectorParameters(parser.get<string>("dp"), detectorParams);
		if (!readOk) 
		{
			cerr << "Invalid detector parameters file" << endl;
			return 0;
		}
	}
	detectorParams->cornerRefinementMethod = aruco::CORNER_REFINE_SUBPIX; // do corner refinement in markers

	int camId = parser.get<int>("ci");

	String video;
	if (parser.has("v")) 
	{
		video = parser.get<String>("v");
	}

	if (!parser.check()) 
	{
		parser.printErrors();
		return 0;
	}

	Ptr<aruco::Dictionary> dictionary = aruco::getPredefinedDictionary(aruco::PREDEFINED_DICTIONARY_NAME(dictionaryId));

	Mat camMatrix, distCoeffs;
	if (estimatePose) 
	{
		bool readOk = readCameraParameters(parser.get<string>("c"), camMatrix, distCoeffs);
		if (!readOk) 
		{
			cerr << "Invalid camera file" << endl;
			return 0;
		}
	}

	VideoCapture inputVideo;
	int waitTime;
	if (!video.empty()) 
	{
		inputVideo.open(video);
		waitTime = 0;
	}
	else 
	{
		inputVideo.open(camId);
		waitTime = 10;
	}

	double totalTime = 0;
	int totalIterations = 0;

	// TODO
	bool hasArVideo = false;
	cv::VideoCapture ARVideo;
	cv::Mat ARFrame, ARDestFrame, blankFrame, maskFrame;
	vector<cv::Point2f> sourcePoints, destPoints;
	vector<cv::Mat> warp_matrices;
	int srcWidth, srcHeight;

	std::string mode = parser.get<std::string>("m");
	std::string path = parser.get<std::string>("p");
	if (mode == "video")
	{
		ARVideo.open(path);
		if (ARVideo.isOpened() == true)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			ARVideo.read(ARFrame);
			
			blankFrame = cv::Mat::zeros(ARFrame.size(), ARFrame.type());
			cv::bitwise_not(blankFrame, blankFrame);
			
			srcWidth = ARFrame.size().width;
			srcHeight = ARFrame.size().height;

			hasArVideo = true;
			sourcePoints.push_back(cv::Point2f(0, 0));
			sourcePoints.push_back(cv::Point2f(srcWidth, 0));
			sourcePoints.push_back(cv::Point2f(srcWidth, srcHeight));
			sourcePoints.push_back(cv::Point2f(0, srcHeight));
		}
	}
	

	while (inputVideo.grab()) 
	{
		Mat image, imageCopy;
		inputVideo.retrieve(image);

		vector< int > ids;
		vector< vector< Point2f > > corners, rejected;
		vector< Vec3d > rvecs, tvecs;

		// detect markers and estimate pose
		aruco::detectMarkers(image, dictionary, corners, ids, detectorParams, rejected);
		if (estimatePose && ids.size() > 0)
		{
			aruco::estimatePoseSingleMarkers(corners, markerLength, camMatrix, distCoeffs, rvecs, tvecs);
		}
			

		// draw results
		image.copyTo(imageCopy);
		if (ids.size() > 0) 
		{
			aruco::drawDetectedMarkers(imageCopy, corners, ids);

			if (estimatePose) 
			{
				for (unsigned int i = 0; i < ids.size(); i++)
					aruco::drawAxis(imageCopy, camMatrix, distCoeffs, rvecs[i], tvecs[i], markerLength * 0.5f);
			}
		}

		if (showRejected && rejected.size() > 0)
		{
			aruco::drawDetectedMarkers(imageCopy, rejected, noArray(), Scalar(100, 0, 255));
		}

		if (hasArVideo == true && ids.size() > 0)
		{
			if (ARVideo.grab())
			{
				ARVideo.retrieve(ARFrame);
			}

			for (int i = 0; i < ids.size(); i++)
			{
				if (ids[i] == 3)
				{
					cv::Mat warp_matrix;
					for (int j = 0; j < sourcePoints.size(); j++)
					{
						destPoints.push_back(corners[i][j]);
					}
					warp_matrix = cv::getPerspectiveTransform(sourcePoints, destPoints);

					cv::warpPerspective(ARFrame, ARDestFrame, warp_matrix, image.size());
					
					
					cv::warpPerspective(blankFrame, maskFrame, warp_matrix, image.size());
					cv::bitwise_not(maskFrame, maskFrame);
					cv::bitwise_and(maskFrame, image, maskFrame);

					cv::bitwise_or(ARDestFrame, maskFrame, maskFrame);
					

					cv::imshow("Source", ARFrame);
					cv::imshow("Dest", ARDestFrame);

					cv::bitwise_and(ARDestFrame, image, image);
					cv::imshow("Result", maskFrame);

					destPoints.clear();
				}
			}

		}
		else
		{
			cv::destroyWindow("Source");
			cv::destroyWindow("Dest");
			cv::destroyWindow("Result");
		}

		
		imshow("out", imageCopy);
		char key = (char)waitKey(waitTime);
		if (key == 27) break;
	}

	return 0;
}
