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
    //const char* filepath = "C:/Users/evali/Pictures/Lego_TestPlate_GoodLighting2.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/Lego_TestPlate_GoodLighting2_testrow.jpg";
    const char* filepath = "C:/Users/evali/Pictures/WebcamLenovo_simple.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/Webcam_Envy360_complex.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/WebcamLenovo_glare_skew.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/Phone_Rosia_glare.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/Phone_Rosia_no_glare.jpg";

    cv::Mat notBlured = cv::imread(filepath, cv::IMREAD_COLOR_BGR);
    cv::Mat small; 
    cv::Mat image;
    cv::Mat greyScale, green;
    cv::Mat lumosity, axis, by; 
    cv::Mat sobelx, sobely, gradient;
    cv::Mat gradient_abs_l;
    cv::Mat gradient_abs_a;
    cv::Mat gradient_abs_b;
    cv::Mat gradient_abs_grey, gradient_abs_green;;

    cv::resize(notBlured,small, cv::Size(), 0.35, 0.35, cv::INTER_LINEAR);
    cv::bilateralFilter(small, image, 30, 60, 15);
    cv::cvtColor(image, greyScale, cv::COLOR_BGR2GRAY);

    std::unique_ptr<ImageProcessor> preProcessor = std::make_unique<ImageProcessor>(image);
    lumosity = preProcessor->getLuminance();
    axis = preProcessor->getAxis();
    by = preProcessor->getBY();
    green = preProcessor->getGreen();

    preProcessor->display(image, "Original");
    preProcessor->display(greyScale, "Greyscale");
    preProcessor->display(lumosity, "Luminance");

    //preProcessor->displayRGBChannels();
    //preProcessor->displayHSVChannels();
    //preProcessor->displayLABChannels();
    //preProcessor->displayLUVChannels();
    cv::Mat processed = preProcessor->naiveRgbColourSpaceReduction(preProcessor->getMainImage(), 32);
    cv::cvtColor(processed, greyScale, cv::COLOR_BGR2GRAY);
    preProcessor->sharpen2Dedges(greyScale);
    preProcessor->display(greyScale, "Greyscale");

    // Apply Sobel operator
    cv::Sobel(green, sobelx, CV_64F, 1, 0, 3);
    cv::Sobel(green, sobely, CV_64F, 0, 1, 3);
    // Compute gradient magnitude
    cv::magnitude(sobelx, sobely, gradient);
    // Convert to 8-bit image
    cv::convertScaleAbs(gradient, gradient_abs_green);
    // Display result
    cv::imshow("Green - Sobel Edge Detection", gradient_abs_green);
    cv::waitKey(0);

    // Apply Sobel operator
    cv::Sobel(greyScale, sobelx, CV_64F, 1, 0, 3);
    cv::Sobel(greyScale, sobely, CV_64F, 0, 1, 3);
    // Compute gradient magnitude
    cv::magnitude(sobelx, sobely, gradient);
    // Convert to 8-bit image
    cv::convertScaleAbs(gradient, gradient_abs_grey);
    // Display result
    cv::imshow("Grey - Sobel Edge Detection", gradient_abs_grey);
    cv::waitKey(0);

    // Apply Sobel operator
    cv::Sobel(lumosity, sobelx, CV_64F, 1, 0, 3);
    cv::Sobel(lumosity, sobely, CV_64F, 0, 1, 3);
    // Compute gradient magnitude
    cv::magnitude(sobelx, sobely, gradient);
    // Convert to 8-bit image
    cv::convertScaleAbs(gradient, gradient_abs_l);
    // Display result
    cv::imshow("Lum - Sobel Edge Detection", gradient_abs_l);
    cv::waitKey(0);

    // Apply Sobel operator
    cv::Sobel(axis, sobelx, CV_64F, 1, 0, 3);
    cv::Sobel(axis, sobely, CV_64F, 0, 1, 3);
    // Compute gradient magnitude
    cv::magnitude(sobelx, sobely, gradient);
    // Convert to 8-bit image
    cv::convertScaleAbs(gradient, gradient_abs_a);
    // Display result
    cv::imshow("Axi - Sobel Edge Detection", gradient_abs_a);
    cv::waitKey(0);

    // Apply Sobel operator
    cv::Sobel(by, sobelx, CV_64F, 1, 0, 3);
    cv::Sobel(by, sobely, CV_64F, 0, 1, 3);
    // Compute gradient magnitude
    cv::magnitude(sobelx, sobely, gradient);
    // Convert to 8-bit image
    cv::convertScaleAbs(gradient, gradient_abs_b);
    // Display result
    cv::imshow("BY - Sobel Edge Detection", gradient_abs_b);
    cv::waitKey(0);

    cv::Mat allEdgesAdded;
    cv::add(gradient_abs_l, gradient_abs_green, allEdgesAdded);
    cv::add(gradient_abs_b, allEdgesAdded, allEdgesAdded);
    cv::add(allEdgesAdded, gradient_abs_a, allEdgesAdded);
    cv::add(allEdgesAdded, gradient_abs_a, allEdgesAdded);
    //cv::add(gradient_abs_grey, allEdgesAdded, allEdgesAdded);
    //cv::add(gradient_abs_green, allEdgesAdded, allEdgesAdded);

    cv::imshow("All Edges Added up", gradient_abs_b);
    cv::waitKey(0);

    preProcessor->display(processed, "Filtered");
    //preProcessor->updateHSVChannelsWithProcessed(processed);
    //preProcessor->displayRGBChannels();
    //preProcessor->displayHSVChannels();
    //preProcessor->displayLABChannels();
    //preProcessor->displayLUVChannels();

    cout << "----------------------------End--------------------------------" << endl;
    return 0;
}
