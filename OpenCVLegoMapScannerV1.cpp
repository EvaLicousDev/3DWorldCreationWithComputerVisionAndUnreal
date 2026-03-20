// OpenCVLegoMapScannerV1.cpp : Defines the entry point for the application.

//Project includes
#include "OpenCVLegoMapScannerV1.h"
#include "src/ProcessingLib/ImageProcessor.h"

//std lib includes
#include <memory>

#include "src/ProcessingLib/ImageProcessor.h"
#include "src/ProcessingLib/BrickColourClassifier.h"
#include "src/ProcessingLib/ColourDetector.h"

using namespace std;
using namespace cv;
using namespace PreProcessor;


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
    //const char* filepath = "C:/Users/evali/Pictures/HDR_MAP2.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/HDR_MAP3.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/HDR_MAP4.jpg";
    const char* filepath = "C:/Users/evali/Pictures/HDR_MAP5.jpg";

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
    //const char* histPath = "C:/Users/evali/Pictures/HDR_DAY_Red.jpg";
    //const char* histPath = "C:/Users/evali/Pictures/HDR_DAY_2_Purple.jpg";
    //const char* histPath = "C:/Users/evali/Pictures/HDR_DAY_2_LightGreen.jpg";
    //const char* histPath = "C:/Users/evali/Pictures/HDR_DAY_2_Pink.jpg";
    //const char* histPath = "C:/Users/evali/Pictures/HDR_DAY_2_Pink2.jpg";
    //const char* histPath = "C:/Users/evali/Pictures/HDR_DAY_2_DarkBlue.jpg";
    //const char* histPath = "C:/Users/evali/Pictures/HDR_DAY_2_Head.jpg";


    //Get the desired image from location 
    //cv::namedWindow("Image to process NO BLUR");

    //---------------------------------------------------------
    bool demo = true; 
    //---------------------------------------------------------

    cv::Mat imageToProcess = cv::imread(filepath, cv::ImreadModes::IMREAD_COLOR_BGR); 
    
    //resize image and set up processor
    auto processor = ImageProcessor(imageToProcess);
    imageToProcess = processor.reseizeImage(imageToProcess);

    if (demo) cv::imshow("currently processing this image", imageToProcess);
    if (demo) cv::waitKey(0);

    //prepare blurred
    cv::Mat blurr; 
    medianBlur(imageToProcess, blurr, 9); 
    
    // We find the lighter colours in the Red channel to seperate light green and yellow form dark Green before looking 
    // for the dark green corner pieces
    cv::Mat splitBGR[channels];
    cv::Mat meanShift; 
    cv::split(imageToProcess, splitBGR);
    cv::Mat higherRedValueMask = splitBGR[redOrBlueYellow]; //we don't use a pointer bc we want to preserve imageToProcess
    threshold(higherRedValueMask, higherRedValueMask, 50, 255, cv::THRESH_BINARY_INV); 
    medianBlur(higherRedValueMask, higherRedValueMask, 9);
    threshold(higherRedValueMask, higherRedValueMask, 0, 100, cv::THRESH_BINARY);

    //creating first mask - only pixels where the red value is < 50, so perceptually green and blue hues
    cv::Mat greenBlueHueAreas;
    imageToProcess.copyTo(greenBlueHueAreas, higherRedValueMask); 

    if (demo) imshow("Low Red Values Mask", greenBlueHueAreas);
    if (demo) waitKey(0);

    // at this point, depending on the lighting, we still have a lot of black tray in our image
    // we now use the green channel to reduce the pixels evaluated for feature detection further
    cv::Mat splitGreenMask[3];
    cv::split(greenBlueHueAreas, splitGreenMask);
    cv::Mat noLowGreenValuesMask = splitGreenMask[greenOrAxis]; 
    normalize(noLowGreenValuesMask, noLowGreenValuesMask, 0, 255, NORM_MINMAX); //improve uneven light conditions
    cv::medianBlur(noLowGreenValuesMask, noLowGreenValuesMask, 9); 

    cv::threshold(noLowGreenValuesMask, noLowGreenValuesMask, 50, 255, THRESH_BINARY);
    if (demo) imshow("Green Channel value 50 - 255", noLowGreenValuesMask);
    if (demo) waitKey(0);

    cv::Mat noLowRedGreen; 
    greenBlueHueAreas.copyTo(noLowRedGreen, noLowGreenValuesMask); 
    normalize(noLowRedGreen, noLowRedGreen, 0, 255, NORM_MINMAX); //improve uneven light conditions

    if (demo) imshow("Presumed Green areas", noLowRedGreen);
    if (demo) waitKey(0);

    cv::Mat mserOutMask;
    cv::Mat greenChanFinal[3]; 
    cv::split(noLowRedGreen, greenChanFinal);
    cv::Mat green = greenChanFinal[greenOrAxis]; 

    auto presumedROI = processor.useContoursToFindCorners(imageToProcess, noLowGreenValuesMask, demo, demo);

    if (!presumedROI.empty())
    {   
        // apply transform now that we know corners of plate
        // we first need the matrix
        // we use the largest rectangle as base  
        cv::Mat corrected = cv::Mat::zeros(imageToProcess.cols, imageToProcess.rows, CV_32F); 
        auto plateRectangle = processor.getPlateRectangle();
        auto destinationPoints = std::vector<cv::Point2f>();
        if (auto plate = plateRectangle.lock())
        {
            cv::Point2f topLeft(cv::Point2f(plate->tl()));
            cv::Point2f botLeft(cv::Point2f(static_cast<float>(plate->tl().x), static_cast<float>(plate->br().y)));
            cv::Point2f botRight(cv::Point2f(plate->br()));
            cv::Point2f topRight(cv::Point2f(static_cast<float>(plate->tl().x + plate->width), static_cast<float>(plate->tl().y)));
            destinationPoints.emplace_back(topLeft);
            destinationPoints.emplace_back(topRight);
            destinationPoints.emplace_back(botRight);
            destinationPoints.emplace_back(botLeft);
        }

        if (!destinationPoints.empty())
        {
            auto matrix = cv::getPerspectiveTransform(presumedROI, destinationPoints);
            cv::warpPerspective(imageToProcess, corrected, matrix, imageToProcess.size());
        }
 
        demo = true;
        if (demo) imshow("Corrected", corrected);
        if (demo) waitKey(0);

        /*
        cv::Mat  unprocessedROI = imageToProcess(plate);
        auto thresholdingROI = cv::Rect(plate.tl().x, 0, plate.width, plate.tl().y);
        cv::Mat exampleBricks = imageToProcess(thresholdingROI);

        if (demo)
        {
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
                bool isBrickShaped = (rectangleInstance.height >= rectangleInstance.width * 2);
                if ((((rectangleInstance.width + rectangleInstance.height) != 0) && isBrickShaped))
                {
                    bricks.emplace_back(rectangleInstance);
                }
            }
        }

        mserOutMask = processor.getMSERMask(unprocessedROI);
        cv::medianBlur(mserOutMask, mserOutMask, 3);
        processor.getContourData(mserOutMask, true);

        if (demo)
        {
            imshow("MSER contours", processor.getDrawnContours());
            imshow("Feature detection", mserOutMask);
            cv::waitKey(0);
        }

        bool showProjection = true;
        for (auto brick : bricks)
        {
            auto brickMat = exampleBricks(brick);
            auto pixels = processor.backprojectHistogram(unprocessedROI, brickMat, 180);
            cv::threshold(pixels, pixels, 200, 255, THRESH_BINARY);
            cv::Mat showPixls;
            unprocessedROI.copyTo(showPixls, pixels);

            if (showProjection)
            {
                auto detector = AbsColourDistance::ColourDetector(); 
                auto brickColour = detector.getBrickApproximation(brickMat); 

                auto name = std::string(getBrickColour(brickColour));
                int margin = 5;
                int base = 0; 
                cv::Size textSize = cv::getTextSize(name, cv::FONT_HERSHEY_COMPLEX, 2, 2, &base); 
                auto text = cv::Point(showPixls.cols-textSize.width-margin, textSize.height + margin);  
                cv::putText(showPixls, name, text, cv::FONT_HERSHEY_COMPLEX, 2, CV_RGB(255, 50, 100), 2);
                cv::imshow("Brick", brickMat);
                cv::imshow("Assumed area", showPixls);
                cv::waitKey(2);
            }

            cv::Mat res;
            cv::matchTemplate(unprocessedROI, brickMat, res, TemplateMatchModes::TM_SQDIFF_NORMED);
            //cv::threshold(res, res, 0, 100, THRESH_MASK);
            imshow("matched", res);
            cv::waitKey(2);
            }
            */
        
        
    }

    // for (auto brick : bricks)
    // {
    //     auto brickMat = exampleBricksBlurred(brick);
    //     //get square in middle of brick
    //     auto newX = brick.width / 4; 
    //     auto newY = (brick.height / 8)*3;
    //     auto sample = cv::Rect(newX, newY, (brick.width / 2), (brick.height / 4));
    //     cv::Mat smallSample = brickMat(sample); 
    //
    //     cv::Mat showPixles; 
    //     cv::Mat colourHistorgram;
    //     cv::Mat projection;
    //
    //     const float hranges[2] = {0.0, 255.0};
    //     const float* ranges[3] = {hranges, hranges, hranges};
    //     int chan[3] = {0,1,2};
    //     int histSz = 255; 
    //     cv::calcHist(&smallSample, 1, chan, cv::Mat(), colourHistorgram, 2, &histSz, ranges); 
    //     cv::normalize(colourHistorgram, colourHistorgram, 0, 255, NORM_MINMAX);
    //     cv::calcBackProject(&legoROI, 1, chan, colourHistorgram, projection, ranges); 
    //     cv::threshold(projection, projection, 100, 255, THRESH_BINARY);
    //
    //     legoROI.copyTo(showPixles, projection);
    //     imshow("Colour Hist projection", showPixles); 
    //     imshow("Sample", smallSample);
    //     cv::waitKey(0);
    // }
    cv::waitKey(0);

    cout << "----------------------------End--------------------------------" << endl;
    return 0;
}
