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
    // Saturation for initial detection of plate
    // GREEN channel + LAB  for colour identification
    //Take image as is and get the Value channel or greyscale


    std::cout << "Starting Preprocessing" << std::endl;
    const char* filepath = "C:/Users/evali/Pictures/Lego_TestPlate_GoodLighting2.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/Lego_TestPlate_GoodLighting2_testrow.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/WebcamLenovo_simple.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/Webcam_Envy360_complex.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/WebcamLenovo_glare_skew.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/Phone_Rosia_glare.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/Phone_Rosia_no_glare.jpg";

    cv::Mat notBlured = cv::imread(filepath);
    cv::Mat small; 
    cv::Mat image;
    cv::resize(notBlured,small, cv::Size(), 0.35, 0.35, cv::INTER_LINEAR);
    cv::bilateralFilter(small, image, 30, 60, 15);

    //cv::Mat image = cv::imread(filepath);

    std::unique_ptr<ImageProcessor> preProcessor = std::make_unique<ImageProcessor>(image);
    preProcessor->displayRGBChannels();
    preProcessor->displayHSVChannels();
    preProcessor->displayLABChannels();
    preProcessor->displayLUVChannels();
    cv::Mat processed = preProcessor->naiveRgbColourSpaceReduction(preProcessor->getMainImage(), 32);
    preProcessor->display(processed);
    preProcessor->updateHSVChannelsWithProcessed(processed);
    preProcessor->displayRGBChannels();
    preProcessor->displayHSVChannels();
    preProcessor->displayLABChannels();
    preProcessor->displayLUVChannels();

    cout << "----------------------------End--------------------------------" << endl;
    return 0;
}
