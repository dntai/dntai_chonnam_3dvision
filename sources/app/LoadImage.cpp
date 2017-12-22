#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>

using namespace cv;

int main(int argc, char *argv[])
{
	String fname; 
	Mat img;

	fname = "Data//lena_std.tif";
	if (argc == 2)
	{
		fname = String(argv[1]);
	}

	img = imread(fname, IMREAD_COLOR);

	imshow("Load Image", img);
	
	std::cout << "Press any key to exit..." << std::endl;
	cv::waitKey(0);

	return EXIT_SUCCESS;
}

