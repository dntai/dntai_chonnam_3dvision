#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include <opencv2/calib3d.hpp>
#include <chrono>
#include <thread>

#include <camcalib\CameraCalibrator.h>
using namespace Vision3D::CamCalib;

using namespace cv;

int main(int argc, char *argv[])
{
	int key, cnt;
	Mat frame, gray, gray1, frame1, tmp;
	int width, height;
	VideoCapture reader(0);

	// Check camera open
	if (!reader.isOpened())
	{
		std::cout << "Load camera failed!" << std::endl;
		return EXIT_FAILURE;
	}

	// Read information camera
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	for (cnt = 1; !reader.read(frame) && cnt <= 10; cnt++)
	{
		std::cout << "Retry read camera information " << cnt << " time(s)!" << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	if (cnt > 10)
	{
		std::cout << "Read camera information failed!" << std::endl;
		return EXIT_FAILURE;
	}

	width = frame.size().width;
	height = frame.size().height;

	int success;
	CameraCalibrator cameraCalibrator;
	std::vector<cv::Point2f> imageCorners;

	// Process camera
	int nsave = 1;
	char name[255];
	for (cnt = 1, key = -1; reader.read(frame) == true && key != 27; cnt++)
	{
		std::cout << "Read Frame #" << cnt << std::endl;

		success = cameraCalibrator.findChessboardImage(frame, cv::Size(6, 4), imageCorners);

		if (success)
		{
			frame.copyTo(tmp);
			cv::drawChessboardCorners(tmp, cv::Size(6, 4), imageCorners, true);
			cv::imshow("Video", tmp);
			key = cv::waitKey(0);
			if (key == 32)
			{
				sprintf(name, "DATA\\boards1\\board%d.png", nsave);
				cv::imwrite(name, frame);
				nsave++;
			}
		}
		else
		{
			cv::imshow("Video", frame);
			key = cv::waitKey(1);
		}
	}

	// system("PAUSE");

	return EXIT_SUCCESS;
}