#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>

using namespace cv;

int main(int argc, char *argv[])
{
	Mat frame;
	VideoCapture reader(0);

	cv::namedWindow("Video", CV_WINDOW_NORMAL | WINDOW_KEEPRATIO | CV_GUI_NORMAL);
	cv::resizeWindow("Video", 512, 512);

	if (!reader.isOpened())
	{
		std::cout << "Load camera failed!" << std::endl;
		return EXIT_FAILURE;
	}

	for (int cnt = 1; reader.read(frame) == true; cnt++)
	{
		std::cout << "Read Frame #" << cnt << std::endl;
		cv::imshow("Video", frame);
		cv::imwrite("a.png", frame);
		if (cv::waitKey(1) == 27) break;
	}

	return EXIT_SUCCESS;
}