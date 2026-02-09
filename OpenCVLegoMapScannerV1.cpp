// OpenCVLegoMapScannerV1.cpp : Defines the entry point for the application.

//Project includes
#include "OpenCVLegoMapScannerV1.h"
#include "src/ProcessingLib/ImageProcessor.h"

//std lib includes
#include <memory>

using namespace std;
using namespace cv;
using namespace PreProcessor;

int main()
{
	std::cout << "Starting Preprocessing" << std::endl;
	const char* filepath = "";
	cv::Mat image = cv::imread(filepath);

	std::unique_ptr<ImageProcessor> preProcessor = std::make_unique<ImageProcessor>(image);
	cv::Mat processed = preProcessor->naiveRgbColourSpaceReduction(preProcessor->getMainImage(), 128);
	processed = preProcessor->sharpen2Dedges(processed); 
	preProcessor->display(processed, 500);

	cout << "---------------------End--------------------------------" << endl;
	return 0;
}
