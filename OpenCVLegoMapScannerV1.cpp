// OpenCVLegoMapScannerV1.cpp : Defines the entry point for the application.

//Project includes
#include "OpenCVLegoMapScannerV1.h"
#include "src/ProcessingLib/ImageProcessor.h"

//std lib includes
#include <memory>

#include "src/ProcessingLib/BrickColourClassifier.h"

using namespace std;
using namespace cv;
using namespace PreProcessor;

RNG rng(12345);

int main()
{
    // TAKE LAB CHANNEL AND ITTERATIVELY FIND THRESHOLD until large white rectangle is found
    // From that rectangle we section the image upwards, taking only the width of the white rectangle and moving it up into the colour example space
        // Now we use the BlueYellow channel (because the colour order is based on the *typical order of the B channels values from Dark to light)
        // Iterating over the BlueYellow channel we go form 0-256 until we find a rectangle-ish formation
            // If the formation is square blue and purple were picked up at the same time (Axis channel will confirm which is which)
            // If the formation is rectengular we know we found the first of the two 
                // We then record the lower bound for the first colour (purple) and create a mask for it
                // This mask is then checked every step in a central position until the value of a small selection of pixls are 0 again
                // Then we record the upper bound for that colour
            // As we increase the pixel value upper and lower bounds we repeat this until we have 8 colours thresholded in A & B 

    //SUGGESTED ALG
    /*
     *Pre alg: 
     * 1. Get 5-10 example images of all colours
     * 2. Calculate histograms of isolated colour bricks
     *
     *Main:
     * 1. Find square and apply transform
     * 2. Use B/W bounderies to find threshold area of colours
     *      a. create areas of interest through finding widest bounding box of 4 sided shape
     * 3. Create mask to just use square
     *      a. reduce BGR colour space to 32-64 colours
     *      b. blur image heavily with k-means based on pixle transform and resize
     *      c. use red channel to isolate high green values and create a mask for the background
     *          i. create threshold mask by setting all pixles to either 0 or 255 if value < 25
     *      d. transform image to LAB
     *      e. use histogram back projection to threshold all colours
     *          i. for every colour's bounding box 
     *              1. use A & B channels to calculate likelyhood mask
     *              2. combine likelyhood masks for all colours for both channels
     *      f. Filter through bounding boxes of appropriate size > than min num pixls
     *          i. apply colour lable 1-8
     *      g. return coordinates & shapes of results
     */

    //Blue Yellow Red Purple LightGreen

    /*
     * Colour order at top of board:
     *
     * Purple
     * DarkBlue
     */

    std::cout << "Starting Preprocessing" << std::endl;
    //const char* filepath = "C:/Users/evali/Pictures/Lego_TestPlate_GoodLighting2.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/Lego_TestPlate_GoodLighting2_testrow.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/WebcamLenovo_simple.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/Webcam_Envy360_complex.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/WebcamLenovo_glare_skew.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/Phone_Rosia_glare.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/Phone_Rosia_no_glare.jpg";

    //const char* filepath = "C:/Users/evali/Pictures/HDR.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/HDR_soft.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/HDR_vibrant.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/HDR_distance_zoom.jpg";

    const char* filepath = "C:/Users/evali/Pictures/HDR_night.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/HDR_night2.jpg";

    //const char* filepath = "C:/Users/evali/Pictures/RED_HistogramTestImage.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/ORANGE_HistogramTestImage.jpg";

    //Setup fallback solution: Histogram back projection of prepared sample images for each colour brick

    //Get the desired image from location 
    cv::namedWindow("Image to process NO BLUR");
    cv::Mat blurred; 
    cv::Mat lab;
    cv::Mat labChannels[3];
    cv::Mat bgrChannels[3];

    cv::Mat redYellowOrangeMask; 

    cv::Mat imageToProcess = cv::imread(filepath, cv::ImreadModes::IMREAD_COLOR_BGR); 
    auto processor = ImageProcessor(imageToProcess);
    imageToProcess = processor.reseizeImage(imageToProcess);

    cv::imshow("Image to process NO BLUR", imageToProcess);
    cv::waitKey(0);

    cv::medianBlur(imageToProcess, blurred,9);
    cv::imshow("Image to process Median blur k = 9", blurred);
    cv::waitKey(0);

    cv::split(blurred, bgrChannels);
    for (auto channel : bgrChannels)
    {
        cv::imshow("BGR channel", channel);
        cv::waitKey(0);
    }

    cv::threshold(blurred, redYellowOrangeMask, 200, 255, NORM_MINMAX);
    cv::imshow("RED YELLOW ORANGE candidates", redYellowOrangeMask);
    cv::waitKey(0);

    cv::cvtColor(blurred, lab, COLOR_BGR2Lab);
    cv::split(lab,labChannels); 


    for (int lower = 256; lower > 0; lower-=10)
    {
        std::string frameName = "Lower bound : ";
        frameName.append(std::to_string(lower)); 
        frameName.append(" Upper bound : ");
        frameName.append(std::to_string((lower-10)));
        ImageProcessor::thresholdColourOnChannel(labChannels[0], (lower-10), (lower), frameName.c_str(), true);
    }

    for (int lower = 256; lower > 11; lower -= 10)
    {
        std::string frameName = "Lower bound : ";
        frameName.append(std::to_string(lower));
        frameName.append(" Upper bound : ");
        frameName.append(std::to_string((lower - 10)));
        ImageProcessor::thresholdColourOnChannel(labChannels[2], (lower - 10), (lower), frameName.c_str(), true);
    }

    cout << "----------------------------End--------------------------------" << endl;
    return 0;
}
