// OpenCVLegoMapScannerV1.cpp : Defines the entry point for the application.

//Project includes
#include "OpenCVLegoMapScannerV1.h"
#include "src/ProcessingLib/ImageProcessor.h"

//std lib includes
#include <memory>

#include "src/ProcessingLib/BrickColourClassifier.h"
#include "src/ProcessingLib/ColourDetector.h"

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
    //Example images
    //const char* filepath = "C:/Users/evali/Pictures/HDR_DAY.jpg";
    const char* filepath = "C:/Users/evali/Pictures/HDR_Test.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/HDR_DAY_2.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/HDR_DARK.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/NORM_DAY.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/NORM_DAY2.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/NORM_DARK.jpg";


    //Example bricks
    const char* histPath = "C:/Users/evali/Pictures/HDR_DAY_Red.jpg";
    //const char* histPath = "C:/Users/evali/Pictures/HDR_DAY_2_Purple.jpg";
    //const char* histPath = "C:/Users/evali/Pictures/HDR_DAY_2_LightGreen.jpg";
    //const char* histPath = "C:/Users/evali/Pictures/HDR_DAY_2_Pink.jpg";
    //const char* histPath = "C:/Users/evali/Pictures/HDR_DAY_2_Pink2.jpg";
    //const char* histPath = "C:/Users/evali/Pictures/HDR_DAY_2_DarkBlue.jpg";
    //const char* histPath = "C:/Users/evali/Pictures/HDR_DAY_2_Head.jpg";


    //Get the desired image from location 
    //cv::namedWindow("Image to process NO BLUR");

    cv::Mat blurred; 
    cv::Mat lab;
    cv::Mat labChannels[3];
    cv::Mat bgrChannels[3];
    cv::Mat mserOutMask; 

    cv::Mat redYellowOrangeMask; 
    cv::Mat imageToProcess = cv::imread(filepath, cv::ImreadModes::IMREAD_COLOR_BGR); 

    auto processor = ImageProcessor(imageToProcess);
    imageToProcess = processor.reseizeImage(imageToProcess);
    imshow("image", imageToProcess);

    cv::Mat roi = cv::imread(histPath, cv::ImreadModes::IMREAD_COLOR_BGR);

    cv::medianBlur(imageToProcess, blurred, 9);
    cv::medianBlur(roi, roi, 9);

    auto plate = processor.findRectWithLargestVoliumInGreenChannel(blurred, 100, true);
    rectangle(blurred, plate, CV_RGB(255, 0, 255));

    mserOutMask = processor.getMSERMask(imageToProcess); 
    auto greenMask = processor.getGreenMask(); 
    processor.getContourData(mserOutMask, true);
    imshow("mser controus", processor.getDrawnContours());

    auto rectanglesPtr2 = processor.getRectangles();
    if (rectanglesPtr2 != nullptr && rectanglesPtr2->size() != 0)
    {
        for (auto rectangleInstance : *rectanglesPtr2)
        {
            if ((rectangleInstance.width + rectangleInstance.height) != 0)
            {
                rectangle(blurred, rectangleInstance, CV_RGB(50, 150, 150));
            }
        }
    }
    imshow("mser controus on blurr", blurred);

    waitKey(0);
    if (greenMask != nullptr)
    {
        cv::Mat addedMasks; 
        addedMasks = processor.addGreenAndMSER(*greenMask, mserOutMask);
        medianBlur(addedMasks, addedMasks, 9); 
        imshow("added green and mser mask", addedMasks);

        cv::Mat whiteAreas; 
        blurred.copyTo(whiteAreas, addedMasks); 
        cv::Mat greyMask;
        cvtColor(whiteAreas, greyMask, COLOR_BGR2GRAY);

        processor.getContourData(greyMask, true); 

//        auto whiteRect = processor.findRectWithLongestSide(*processor.getContourPoints(), plate); 
  //      rectangle(blurred, whiteRect, CV_RGB(0, 255, 255), 2);
    //    imshow("With white box", blurred);
      //  waitKey(0);

        cv::Mat soble = processor.applySobel(addedMasks, 5); 
        imshow("soble addedMasks k = 5", soble);

        cv::Mat canny = processor.applyCannyTo1D(addedMasks, 200);
        imshow("canny", canny);

        processor.getContourData(canny, true); 
        imshow("controus", processor.getDrawnContours());


        cv::Rect closestToWhite;
        double distanceToWhite = 10000;
        cv::Scalar white(255, 255, 255);

        auto rectanglesPtr = processor.getRectangles(); 
        if (rectanglesPtr != nullptr && rectanglesPtr->size() != 0)
        {
            for (auto rectangleInstance : *rectanglesPtr )
            {
                if (rectangleInstance.width != 0 && rectangleInstance.height != 0)
                {
                    cv::rectangle(imageToProcess, rectangleInstance, CV_RGB(0, 255, 0), 2);

                    cv::Mat rectanglePxls = whiteAreas(rectangleInstance);
                    cv::Scalar avarageColour = cv::mean(rectanglePxls);
                    double absDistance = cv::norm(white, avarageColour, NORM_L2);
                    if (absDistance < distanceToWhite)
                    {
                        distanceToWhite = absDistance;
                        closestToWhite = rectangleInstance;
                    }
                }
            }
        }

        rectangle(imageToProcess, closestToWhite, CV_RGB(255, 0, 0), 3);

        imshow("Squares", imageToProcess);
        cv::waitKey(0);

        cv::rectangle(imageToProcess, closestToWhite, CV_RGB(255, 255, 0), 2);

        auto roi = cv::Mat(blurred, closestToWhite);
        processor.backprojectHistogram(whiteAreas, roi, 20);
    }


    cv::imshow("Image to process Median blur k = 9", blurred);
    cv::imshow("ROI blurred", roi);
    cv::waitKey(0);

    redYellowOrangeMask = processor.backprojectHistogram(blurred, roi, 150);

    // int thresh = 20; 
    // auto absDifRed = AbsColourDistance::ColourDetector::processChannel(blurred, roi, thresh, BrickCV::LAB_AXIS); 
    // cv::imshow("ROI axis histogram", absDifRed);
    // cv::waitKey(0);

    cv::imshow("backprojected histogram", redYellowOrangeMask);
    cv::waitKey(0);

    cout << "----------------------------End--------------------------------" << endl;
    return 0;
}
