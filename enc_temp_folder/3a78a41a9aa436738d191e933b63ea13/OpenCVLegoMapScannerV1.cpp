// OpenCVLegoMapScannerV1.cpp : Defines the entry point for the application.

//Project includes
#include "OpenCVLegoMapScannerV1.h"
#include "src/ProcessingLib/ImageProcessor.h"
#include "src/ProcessingLib/ImageProcessor.h"
#include "src/ProcessingLib/BrickColourClassifier.h"
#include "src/ProcessingLib/ColourDetector.h"

//std lib includes
#include <memory>
#include <map>


using namespace std;
using namespace cv;
using namespace PreProcessor;


int main()
{
    std::cout << "Starting Preprocessing" << std::endl;
    //Example images
    const char* filepath = "C:/Users/evali/Pictures/HDR_MAP1.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/HDR_MAP2.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/HDR_MAP3.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/HDR_MAP4.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/HDR_MAP5.jpg";

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
    bool demo = false; 
    bool resetLog = true; 
    //---------------------------------------------------------
    // Error log gets errased
    if (resetLog)
    {
        std::ofstream ofs;
        ofs.open(Errors::sc_errorOutputFilePath, std::ofstream::out | std::ofstream::trunc);
        ofs.close();
    }
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

    auto presumedROI = processor.useContoursToFindCorners(imageToProcess, noLowGreenValuesMask, true, true);
    auto plateRectangle = processor.getPlateRectangle();
    auto plate = plateRectangle.lock();

    if (!presumedROI.empty() && plate != nullptr)
    {   
        // apply transform now that we know corners of plate
        // we first need the matrix
        // we use the largest volium rectangle as base  
        cv::Mat corrected = cv::Mat::zeros(imageToProcess.cols, imageToProcess.rows, CV_32F); 
        auto destinationPoints = std::vector<cv::Point2f>();
        cv::Point2f topLeft(cv::Point2f(plate->tl()));
        cv::Point2f botLeft(cv::Point2f(static_cast<float>(plate->tl().x), static_cast<float>(plate->br().y)));
        cv::Point2f botRight(cv::Point2f(plate->br()));
        cv::Point2f topRight(cv::Point2f(static_cast<float>(plate->tl().x + plate->width), static_cast<float>(plate->tl().y)));
        destinationPoints.emplace_back(topLeft);
        destinationPoints.emplace_back(topRight);
        destinationPoints.emplace_back(botRight);
        destinationPoints.emplace_back(botLeft);

        if (!destinationPoints.empty())
        {
            auto matrix = cv::getPerspectiveTransform(presumedROI, destinationPoints);
            cv::warpPerspective(imageToProcess, corrected, matrix, imageToProcess.size());
        }

        // Create two images for further processing
        // 1 for the lego plate, 1 for the row above the plate  
        cv::Mat legoPlate = corrected(*plate);
        auto thresholdingROI = cv::Rect(plate->tl().x, 0, plate->width, plate->tl().y);
        cv::Mat exampleBricks = corrected(thresholdingROI);
        cv::Mat labShades;
        cv::cvtColor(exampleBricks, labShades, COLOR_BGR2Lab);

        processor.setImageOfLego(legoPlate);
        processor.setImageOfBricks(exampleBricks);

        if (demo) cv::imshow("Corrected - should be \'straight\' lego plate for better coordinates.", legoPlate);
        if (demo) cv::imshow("Bricks above board", exampleBricks);
        if (demo) waitKey(0);

        //get the location of our example bricks for backprojection and template matching
        auto bricks = processor.findBrickLocations(exampleBricks, demo); 
        std::sort(bricks.begin(), bricks.end(), [](const cv::Rect& left, const  cv::Rect& right)
            {
                return left.tl().x < right.tl().x;
            });

        // Now we set up the colour detector with the scalars for our colours to get a better euclidian distance value
        auto detector = AbsColourDistance::ColourDetector();
        std::map<BrickColour, cv::Rect> brickColourMap{};
        std::map<BrickColour, cv::Scalar> brickScalarMap{};
        int index = 0; 
        for (const auto brick : bricks)
        {
            auto expectedColour = sc_coloursInUse[index];
            cv::Mat brickMat = exampleBricks(brick); 
            auto colourByDistance = detector.getBrickApproximation(brickMat);

            if (expectedColour != colourByDistance)
            {
                // This error is here primarely for testing and development
                // We do not want to fail here if this happens, just have a way to evaluate how well the HTML colour shades fit
                Errors::ErrorOutput(Errors::EUCLIDIAN_DISTANCE_MISMATCH, "Expected colour", getBrickColour(expectedColour), "matched as", getBrickColour(colourByDistance));
            }

            //add to maps
            auto meanColour = cv::mean(exampleBricks(brick));
            brickScalarMap.emplace(expectedColour,meanColour);
            brickColourMap.emplace(expectedColour, brick); 
            index++; 
        }

        detector.setBrickColourMap(brickScalarMap); 

        if (demo)
        {
            for (const auto [colourName, scalar] : brickScalarMap)
            {
                auto av = cv::Mat(300, 300, CV_8UC3, scalar);
                const char* colour = getBrickColour(colourName);
                string frameName = ("Avarage Colour shade: %s", colour);

                auto projectRange = detector.findPixelsWithColourInRange(legoPlate, scalar); 
                cv::imshow("colour projected on AOI with margin of 0.95 - 1.05", projectRange);
                cv::imshow(frameName.c_str(), av);
                cv::waitKey(0);

            }
        }

        // get Features with MSER ( open CV algorithm that does iterative thresholding and looks for regions that stay together )
        // with access to the contributer verion of open CV ximgproc/segmentation.hpp -> Graphbased segmentation would be an intersting alternative here 
        mserOutMask = processor.getMSERMask(legoPlate, true);
        processor.getContourData(mserOutMask, false);
        cv::Mat contoursMSER = processor.applySobel(mserOutMask, 3); 
        processor.getContourData(contoursMSER, false);

        cv::threshold(contoursMSER, contoursMSER, 0, 255, THRESH_BINARY_INV); 

        // convert image to LAB colour space to prepare height map generation and & colour detection
        cv::Mat legoLAB;
        cv::Mat labChannels[3]; 
        cv::cvtColor(legoPlate, legoLAB, COLOR_BGR2Lab); 
        cv::split(legoLAB, labChannels);
        cv::Mat blueYellow = labChannels[redOrBlueYellow]; 
        cv::Mat map = processor.createMapWithContoursAndLABchannel(blueYellow, contoursMSER);

        if (demo) imshow("MAP", map);
        if (demo) imshow("Feature detection", mserOutMask);
        if (demo) cv::waitKey(0);

        bool showProjection = demo;
        for (auto brick : bricks)
        {
            auto brickMat = exampleBricks(brick);
            auto pixels = processor.backprojectHistogram(legoPlate, brickMat, 150);
            cv::Mat showPixls;
            legoPlate.copyTo(showPixls, pixels);

            cv::Mat res;
            cv::matchTemplate(legoPlate, brickMat, res, TemplateMatchModes::TM_SQDIFF_NORMED);
            imshow("template matching", res);
            cv::waitKey(2);

            if (showProjection)
            {
                auto brickColour = detector.getBrickApproximation(brickMat);

                auto name = std::string(getBrickColour(brickColour));
                int margin = 5;
                int base = 0;
                cv::Size textSize = cv::getTextSize(name, cv::FONT_HERSHEY_COMPLEX, 2, 2, &base);
                auto text = cv::Point(showPixls.cols - textSize.width - margin, textSize.height + margin);
                cv::putText(showPixls, name, text, cv::FONT_HERSHEY_COMPLEX, 2, CV_RGB(255, 50, 100), 2);
                cv::imshow("Brick", brickMat);
                cv::imshow("Assumed area", showPixls);
                cv::waitKey(2);
            }
        }
    }

    cv::waitKey(0);

    cout << "----------------------------End--------------------------------" << endl;
    return 0;
}
