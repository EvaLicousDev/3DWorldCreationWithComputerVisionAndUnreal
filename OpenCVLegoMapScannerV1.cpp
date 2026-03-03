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
     //const char* filepath = "C:/Users/evali/Pictures/HDR_MAP1.jpg";
    const char* filepath = "C:/Users/evali/Pictures/HDR_MAP2.jpg";
     //const char* filepath = "C:/Users/evali/Pictures/HDR_MAP3.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/HDR_MAP4.jpg";

    //const char* filepath = "C:/Users/evali/Pictures/HDR_DAY.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/HDR_Test.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/HDR_DAY_2.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/HDR_DARK.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/HDR_DARK_Test.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/NORM_DAY.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/NORM_DAY2.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/NORM_DARK.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/HDR_CAM2.jpg"


    //Example bricks
    const char* histPath = "C:/Users/evali/Pictures/HDR_DAY_Red.jpg";;
    //const char* histPath = "C:/Users/evali/Pictures/HDR_DAY_2_Purple.jpg";
    //const char* histPath = "C:/Users/evali/Pictures/HDR_DAY_2_LightGreen.jpg";
    //const char* histPath = "C:/Users/evali/Pictures/HDR_DAY_2_Pink.jpg";
    //const char* histPath = "C:/Users/evali/Pictures/HDR_DAY_2_Pink2.jpg";
    //const char* histPath = "C:/Users/evali/Pictures/HDR_DAY_2_DarkBlue.jpg";
    //const char* histPath = "C:/Users/evali/Pictures/HDR_DAY_2_Head.jpg";


    //Get the desired image from location 
    //cv::namedWindow("Image to process NO BLUR");

    bool demo = true; 

    cv::Mat blurred; 
    cv::Mat mserOutMask; 
    // cv::Mat redYellowOrangeMask; 
    cv::Mat imageToProcess = cv::imread(filepath, cv::ImreadModes::IMREAD_COLOR_BGR); 

    auto processor = ImageProcessor(imageToProcess);
    imageToProcess = processor.reseizeImage(imageToProcess);
    imshow("image", imageToProcess);

    cv::Mat roi = cv::imread(histPath, cv::ImreadModes::IMREAD_COLOR_BGR);

    cv::medianBlur(imageToProcess, blurred, 9);
    cv::medianBlur(roi, roi, 9);

    //remove the table from image
    auto blackTray = processor.findBlackBG(blurred, true);
    cv::Mat blurredROI = blurred(blackTray); 
    cv::Mat unprocessedROI = imageToProcess(blackTray);
    imshow("blurredROI", blurredROI);
    cv::waitKey(0); 
    
    //find lego
    auto plate = processor.findLegoWithThresholdingMask(blurredROI, 150, true);
    cv::Mat blurredROIDet;
    blurredROI.copyTo(blurredROIDet);
    auto upper = processor.getLegoPXMask();

    if (demo)
    {
        rectangle(blurredROIDet, plate, CV_RGB(255, 0, 255));
        imshow("Detected Board", blurredROIDet);
        waitKey(0);
    }

    auto thresholdingROI = cv::Rect(plate.tl().x, 0, plate.width, plate.tl().y);
    cv::Mat legoROI = blurredROI(plate);
    cv::Mat exampleBricks = unprocessedROI(thresholdingROI);
    cv::Mat exampleBricksBlurred = blurredROI(thresholdingROI);

    bool failedFirstTime = false; 
    cv::Mat unprocessedROI2;
    if (exampleBricks.cols == 0 || exampleBricks.rows == 0)
    {
        // Fallback 1 - Theory:
        // Most cameras pick up green well even in low light conditions
        // Therefor we use the green channel in the RGB image of the unprocessedROI
        failedFirstTime = true; 

        cv::Mat channels[3];
        split(unprocessedROI, channels); 
        cv::Mat greenThresh; 
        medianBlur(channels[1], channels[1], 9); 
        cv::threshold(channels[1], greenThresh, 50, 255, THRESH_BINARY); 

        auto edges = processor.applySobel(greenThresh);
        processor.getContourData(edges, true);
        auto contourPoints = processor.getContourPoints();
        auto largest = processor.findRectWithLargestVolium(*processor.getRectangles());

        thresholdingROI = cv::Rect(largest.tl().x, 0, largest.width, largest.tl().y);
        legoROI = blurredROI(largest);
        exampleBricks = unprocessedROI(thresholdingROI);
        exampleBricksBlurred = blurredROI(thresholdingROI);
        unprocessedROI2 = unprocessedROI(largest);
        upper = processor.getLegoPXMask();

        if (demo)
        {
            rectangle(unprocessedROI, largest, CV_RGB(255, 255, 0), 2);
            imshow("mask faulty detection - green thresh", unprocessedROI);
            cv::waitKey(0);
        }
    }
    
    if (demo)
    {
        imshow("PLate mask", legoROI);
        imshow("Thresh area", exampleBricks);
        waitKey(0);
    }

    mserOutMask = processor.getMSERMask(exampleBricks);
    processor.getContourData(mserOutMask, true);
    auto rectanglesPtr2 = processor.getRectangles();

    //find bricks
    auto bricks = std::vector<cv::Rect>();
    if (rectanglesPtr2 != nullptr && rectanglesPtr2->size() != 0)
    {
        for (auto rectangleInstance : *rectanglesPtr2)
        {
            bool isBrickShaped = (rectangleInstance.height >= rectangleInstance.width*2); 
            if ((((rectangleInstance.width + rectangleInstance.height) != 0) && isBrickShaped))
            {
                if (demo) rectangle(exampleBricksBlurred, rectangleInstance, CV_RGB(50, 150, 150), 2);
                bricks.emplace_back(rectangleInstance);
            }
        }
    }

    if (demo)
    {
        imshow("Bricks", exampleBricksBlurred);
        waitKey(0);
    }

    failedFirstTime ? unprocessedROI = unprocessedROI2 : unprocessedROI = unprocessedROI; 
    mserOutMask = processor.getMSERMask(unprocessedROI); 
    cv::medianBlur(mserOutMask, mserOutMask, 3); 
    imshow("Feature detection", mserOutMask);

    bool showProjection = true; 
    for (auto brick : bricks)
    {
        auto brickMat = exampleBricksBlurred(brick); 
        auto pixels = processor.backprojectHistogram(legoROI, brickMat, 50);
        cv::threshold(pixels, pixels, 100, 255, THRESH_BINARY);

        cv::imshow("Assumed area", pixels);
        cv::waitKey(0);
    }

    cv::waitKey(0);

    cout << "----------------------------End--------------------------------" << endl;
    return 0;
}
