#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>

using namespace cv;

int main(int argc, char *argv[])
{
	String fname;
	Mat frame;
	VideoCapture reader;

	fname = "Data//demo.mp4";
	if (argc == 2)
	{
		fname = String(argv[1]);
	}

	reader.open(fname);
	if (!reader.isOpened())
	{
		std::cout << "Load video failed: " << fname << "!" << std::endl;
		return EXIT_FAILURE;
	}

	for (int cnt = 1; reader.read(frame) == true; cnt++)
	{
		std::cout << "Read Frame #" << cnt << std::endl;
		cv::imshow("Video", frame);
		if (cv::waitKey(5) == 27) break;
	}

	system("PAUSE");

	return EXIT_SUCCESS;
}