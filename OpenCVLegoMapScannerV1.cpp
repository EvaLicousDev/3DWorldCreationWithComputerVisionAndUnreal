//Project includes
#include "OpenCVLegoMapScannerV1.h"
#include "src/ProcessingLib/ImageProcessor.h"
#include "src/ProcessingLib/ImageProcessor.h"
#include "src/ProcessingLib/BrickColourClassifier.h"
#include "src/ProcessingLib/ColourDetector.h"
#include "src/ProcessingLib/ImageReader.h"

#include <filesystem>
#include <memory>
#include <map>

using namespace std;
using namespace cv;
using namespace PreProcessor;

// Image directory defined in CMake Lists file
static const std::string sc_imageFilesPattern = std::string(IMAGE_DIR) + "/*.jpg";


// IMPORTANT INFORMATION:
// This code works on the predefined range of colours in BrickCVEnum/BrickColourEnum -> sc_coloursInUse
// That line needs to match the physical plate above the map area with the example colours from left to right
int main()
{
    std::cout << "--------------------------------------------------------------------------------------" << std::endl;
    std::cout << "---------------------------OPENCV LEGO MAP READER STARTED-----------------------------" << std::endl;
    std::cout << "--------------------------------------------------------------------------------------" << std::endl;
    
    //---------------------------------------------------------
    // Settings
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
    //Load image files
    ImageReader fileReader{};
    fileReader.readImages(sc_imageFilesPattern.c_str());
    auto images = fileReader.getImages(); 

    //---------------------------------------------------------
    //Begin main processing loop - try and process an image and go to the next one if that fails
    for(auto imagePtr : images)
    {
        demo = false; 
        if (auto strongPtr = imagePtr.lock())
        {
            cv::Mat imageToProcess = *strongPtr; 
            if (imageToProcess.empty())
            {
                ErrorOutput(Errors::BrickCVErrors::IMAGE_WASNT_READ_RIGHT, "Breakpoint and check why the image was empty!");
                continue; //go to next image
            }

            //resize image for display and processing and set up processor
            auto processor = ImageProcessor(imageToProcess);
            imageToProcess = processor.reseizeImage(imageToProcess);

            if (demo) cv::imshow("currently processing this image", imageToProcess);
            if (demo) cv::waitKey(0);

            //---------------------------------------------------------
            //Find the inital region of interest with simple thresholding

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

            //---------------------------------------------------------
            // Find points to apply transform on ROI to offset the camera angle and distortion
            // For this we go over all the contours, create a bounding box around the largest
            // and use the center of said bounding box to find the outer corners of the lego plate
            auto presumedROI = processor.useContoursToFindCorners(imageToProcess, noLowGreenValuesMask, demo, demo);
            auto plateRectangle = processor.getPlateRectangle();
            auto plate = plateRectangle.lock();

            if (!presumedROI.empty() && plate != nullptr)
            {
                // apply transform now that we know corners of plate
                // to find the transformation matrix we use the largest rectangle we found around the plate as our base
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

                //make plate square now that we have the right area
                resize(legoPlate, legoPlate, cv::Size(500, 500));

                if (demo) cv::imshow("Corrected - should be \'straight\' lego plate for better coordinates.", legoPlate);
                if (demo) cv::imshow("Bricks above board", exampleBricks);
                if (demo) waitKey(0);

                //get the location of our example bricks for backprojection and template matching
                auto bricks = processor.findBrickLocations(exampleBricks, demo);
                std::sort(bricks.begin(), bricks.end(), [](const cv::Rect& left, const  cv::Rect& right)
                    {
                        return left.tl().x < right.tl().x;
                    });

                int numberOfColours = sizeof(sc_coloursInUse) / sizeof(sc_coloursInUse[0]);
                if (bricks.size() != numberOfColours)
                {
                    ErrorOutput(BrickCVErrors::EXAMPLE_BRICKS_ERROR, "Did not correctly identify", std::to_string(numberOfColours).c_str(), "colours in the exampleBricks image.");
                    continue; //this would scilently break the application, process next image instead
                }

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
                    brickScalarMap.emplace(expectedColour, meanColour);
                    brickColourMap.emplace(expectedColour, brick);
                    index++;
                }

                detector.setBrickColourMap(brickScalarMap);

                demo = true;
                std::map<BrickColour, std::shared_ptr<cv::Mat>> projections;
                for (const auto [colourName, scalar] : brickScalarMap)
                {
                    //make image of colour value and use it to identify pixels within threshold
                    auto meanColour = cv::Mat(300, 300, CV_8UC3, scalar);
                    auto projectRange = detector.findPixelsWithColourInRange(legoPlate, scalar, 0.85, 1.15);

                    // make image of brick
                    auto rect = brickColourMap.at(colourName);
                    cv::Mat brickMat = exampleBricks(rect);

                    // use brick image to get template matching 
                    cv::Mat minnimaMostLikely;
                    cv::matchTemplate(legoPlate, brickMat, minnimaMostLikely, TemplateMatchModes::TM_SQDIFF_NORMED);

                    // invert matching info in preperation of merge
                    cv::Mat resultMap = 1.0 - minnimaMostLikely;
                    resultMap.convertTo(resultMap, CV_8UC1, 255);
                    cv::medianBlur(resultMap, resultMap, 9);

                    auto lchHueChannel = detector.findPixelsWithColourInRangeForChannel(legoPlate, scalar, 0.98, 1.02, ChannelType::LCHuv_HUE, false);
                    auto lchCromaChannel = detector.findPixelsWithColourInRangeForChannel(legoPlate, scalar, 0.98, 1.02, ChannelType::LCHuv_CHROMA, false);
                    cv::Mat chromaHue;
                    cv::add(lchHueChannel, lchCromaChannel * 0.3, chromaHue);
                    cv::medianBlur(chromaHue, chromaHue, 9);

                    // ensure images are same size and add
                    cv::resize(lchHueChannel, lchHueChannel, cv::Size(legoPlate.cols, legoPlate.rows));
                    cv::resize(resultMap, resultMap, cv::Size(legoPlate.cols, legoPlate.rows));
                    cv::resize(projectRange, projectRange, cv::Size(legoPlate.cols, legoPlate.rows));
                    if (colourName != BrickColour::WHITE && colourName != BrickColour::BROWN) cv::add(resultMap, chromaHue, resultMap);
                    cv::add(resultMap, projectRange, resultMap);

                    projections.emplace(colourName, std::make_shared<cv::Mat>(resultMap));

                    if (demo)
                    {
                        cv::Mat leg;
                        legoPlate.copyTo(leg);
                        cv::resize(leg, leg, cv::Size(300, 300));

                        cv::Mat res;
                        resultMap.copyTo(res);
                        cv::resize(res,res, cv::Size(300,300));

                        cv::Mat col;
                        meanColour.copyTo(col);
                        cv::resize(col, col, cv::Size(100, 100));

                        cv::Mat brk;
                        brickMat.copyTo(brk);
                        cv::resize(col, col, cv::Size(100, 200));

                        const char* colour = getBrickColour(colourName);
                        string frameName = ("Avarage Colour shade: %s", colour);
                        //cv::imshow("LCHuv test", chromaHue);
                        cv::imshow(frameName.c_str(), res);
                        cv::imshow("scalar", col);
                        cv::imshow("Brick", brk);
                        cv::imshow("Plate", leg);
                        cv::waitKey(0);
                    }
                }
            }
        } 
        else
        {
            Errors::ErrorOutput(Errors::BrickCVErrors::NULL_PTR, "Image pointer could not be converted into a strong reference and had to be skipped");
        }
    }//END main img processing for loop

    //clean up images 
    fileReader.~ImageReader(); 

    std::cout << "--------------------------------------------------------------------------------------" << std::endl;
    std::cout << "---------------------------OPENCV LEGO MAP READER FINISHED----------------------------" << std::endl;
    std::cout << "--------------------------------------------------------------------------------------" << std::endl;
    return 0;
}
