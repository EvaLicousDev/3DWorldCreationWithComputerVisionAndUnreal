//Project includes
#include "OpenCVLegoMapScannerV1.h"
#include "src/ProcessingLib/ImageProcessor.h"
#include "src/ProcessingLib/ImageProcessor.h"
#include "src/ProcessingLib/BrickColourClassifier.h"
#include "src/ProcessingLib/ColourDetector.h"
#include "src/ProcessingLib/ImageReader.h"
#include "src/ProcessingLib/Tests/CVAppOutputTester.h"

#include <filesystem>
#include <memory>
#include <map>
#include <fstream>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;
using namespace ImageProcessing;

// Image directory defined in CMake Lists file
static const std::string sc_imageFilePath = string(IMAGE_DIR);
static const std::string sc_imageFilesPattern = "Y:/LegoImages/*.jpg";
static const std::string sc_testImages        = string("C:/Users/evali/FinalProjectFiles/OpenCVModel/OpenCVLegoMapScannerV1/ImageFolder/HDR_MAP") + "*.jpg";


// IMPORTANT INFORMATION:
// This code works on the predefined range of colours in BrickCVEnum/BrickColourEnum -> sc_coloursInUse
// That line needs to match the physical plate above the map area with the example colours from left to right, starting with white and ending with white
int main()
{
    std::cout << "--------------------------------------------------------------------------------------" << std::endl;
    std::cout << "---------------------------OPENCV LEGO MAP READER STARTED-----------------------------" << std::endl;
    std::cout << "--------------------------------------------------------------------------------------" << std::endl;
    
    //---------------------------------------------------------
    // Settings
    bool debugOrDemoAll     = true; 
    bool resetLog           = true;  // reccomended
    bool createHeightMapPNG = false; // not needed in final product version 1
    bool testCSV            = false;
    bool useTestImages      = false; 

    const char* heightMapOutputPath = "C:/Users/evali/FinalProjectFiles/OpenCVModel/OpenCVLegoMapScannerV1/OutputFolder/heightMap.png";
    const char* textureOutputPath   = "C:/Users/evali/Pictures/TexMap.png";

    double dynThres_LowerBoundry_LAB      = 0.90; 
    double dynThresh_UpperBoundry_LAB     = 1.10;
    double dynThres_LowerBoundry_LCH      = 0.99;
    double dynThresh_UpperBoundry_LCH     = 1.01;
    double weightFactor_ChromaChannel_LCH = 0.3; 

    int outRowsCols = 40;
    const char* outputfileName = "C:/Users/evali/Pictures/heightMapPoints.csv";


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
    auto files = useTestImages ? sc_testImages : sc_imageFilesPattern; 
    auto path = useTestImages ? "C:/Users/evali/FinalProjectFiles/OpenCVModel/OpenCVLegoMapScannerV1/ImageFolder" : sc_imageFilePath;
    std::cout << "Trying to find image files at " << files << std::endl;
    fileReader.readImages(path.c_str(), files.c_str()); //tries finding files for 5 minutes at specified location
    auto images = fileReader.getImages(); 

    //---------------------------------------------------------
    //Begin main processing loop - try and process an image and go to the next one if that fails

    bool success    = false; 
    int imagesTried = 0; 
    for(auto imagePtr : images)
    {
        ++imagesTried; 
        if (auto strongPtr = imagePtr.lock())
        {
            cv::Mat imageToProc = *strongPtr; 
            if (imageToProc.empty())
            {
                ErrorOutput(Errors::BrickCVErrors::IMAGE_WASNT_READ_RIGHT, "Breakpoint and check why the image was empty!");
                continue; //go to next image
            }

            //resize image for display and processing and set up processor
            auto processor = ImageProcessor(imageToProc);
            imageToProc = processor.reseizeImage(imageToProc);

            if (debugOrDemoAll) cv::imshow("currently processing this image", imageToProc);
            if (debugOrDemoAll) cv::waitKey(0);

            //even out luminance channel and create second image where hue & saturation are increased 
            auto retinex   = processor.createRetinex(imageToProc, debugOrDemoAll);
            auto mask      = processor.adaptiveShadowRemovalMask(imageToProc, retinex, 1, debugOrDemoAll); 
            auto corrected = processor.removeShadows(imageToProc, retinex, mask, debugOrDemoAll);

            cv::Mat imagesToProcess[2] = {imageToProc, corrected}; 

            // MAIN BODY
            for (auto imageToProcess : imagesToProcess) 
            {
                //---------------------------------------------------------
                //Find the inital region of interest with simple thresholding
                if (imageToProcess.empty()) continue; 

                // We find the lighter colours in the Red channel to seperate light green and yellow form dark Green before looking 
                // for the dark green corner pieces. Best results are achieved if two rows of lego studs remain dark green on the side
                cv::Mat splitBGR[channels];
                cv::split(imageToProcess, splitBGR);
                cv::Mat higherRedValueMask = splitBGR[redOrBlueYellow];
                higherRedValueMask * 2;
                threshold(higherRedValueMask, higherRedValueMask, 100, 255, cv::THRESH_BINARY_INV);
                medianBlur(higherRedValueMask, higherRedValueMask, 9);

                //creating first mask - only pixels where the red value is < 50, so perceptually green and blue hues
                cv::Mat greenBlueHueAreas;
                imageToProcess.copyTo(greenBlueHueAreas, higherRedValueMask);

                if (debugOrDemoAll) imshow("Low Red Values Mask", greenBlueHueAreas);
                if (debugOrDemoAll) waitKey(0);

                cv::Mat labInterest;
                greenBlueHueAreas* (1, 3, 1); //strengthen green channel - helps get complete plate in uneven lighting
                cv::cvtColor(greenBlueHueAreas, labInterest, COLOR_BGR2Lab);
                cv::Mat splitLAB[channels];
                cv::split(labInterest, splitLAB);
                cv::Mat labAxis = splitLAB[1]; 
                normalize(labAxis, labAxis, 0, 255, NORM_MINMAX); 
                medianBlur(labAxis, labAxis, 9);
                threshold(labAxis, labAxis, 50, 255, cv::THRESH_BINARY_INV);

                if (debugOrDemoAll) imshow("Greenest", labAxis);
                if (debugOrDemoAll) waitKey(0);

                if (debugOrDemoAll)
                {
                    cv::Mat noLowRedGreen;
                    greenBlueHueAreas.copyTo(noLowRedGreen, labAxis);

                    imshow("Presumed Green areas", noLowRedGreen);
                    waitKey(0);
                }

                //---------------------------------------------------------
                // The camera angle is not guaranteed to be pralell to the plate, so we want to achieve a better read of the "square" lego plate
                // This was added after the mid term demo feedback 

                // Find points to apply transform on ROI to offset the camera angle and distortion
                // For this we go over all the contours, create a bounding box around the largest
                // and use the center of said bounding box to find the outer corners of the lego plate

                auto presumedROI = processor.useContoursToFindCorners(imageToProcess, labAxis, debugOrDemoAll, debugOrDemoAll);
                if (presumedROI.empty()) continue;

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
                    cv::Mat legoPlate     = corrected(*plate);
                    auto thresholdingROI  = cv::Rect(plate->tl().x, 0, plate->width, plate->tl().y);
                    cv::Mat exampleBricks = corrected(thresholdingROI);

                    // (Possible) Further work: There could be verification here by detecting cirlces
                    //                          and either counting them or taking the avarage diameter
                    //                          and ensuring the right number of studs are present each row-col
                    //                          It was decided against this as this would introduce 
                    //                          another layer of complexity due to variant light conditions making shape
                    //                          dection skewed by shadows and reflective surface difficult

                    //                          A small skew in the output result does not destroy the experience 
                    //                          when translated into the 3D environment, because we map the output result
                    //                          in relative terms treating pixels as voxel (pixels with volium) 
                    //                          The output texture is mapped in 2.5D onto the landscape through sampling
                    //                          3D points by their 2D x & y coordinates, onto which the texture is streched to fit
                    //                          Therefor using some type of feature detection here could allow us
                    //                          to internally verify the results, but it's abscence won't destroy the 
                    //                          core functionality of this system

                    processor.setImageOfLego(legoPlate);
                    processor.setImageOfBricks(exampleBricks);

                    //make plate 'square' now that we have the right area
                    resize(legoPlate, legoPlate, cv::Size(500, 500));

                    if (debugOrDemoAll) cv::imshow("Corrected - should be \'straight\' lego plate for better coordinates.", legoPlate);
                    if (debugOrDemoAll) cv::imshow("Bricks above board", exampleBricks);
                    if (debugOrDemoAll) waitKey(0);

                    //---------------------------------------------------------------------------------------------
                    //Try and identify all bricks in the example area
                    cv::Mat boostedBricks = processor.boostValue(exampleBricks, debugOrDemoAll);
                    cv::Mat brickExamples[2] = {exampleBricks, boostedBricks}; 
                    int numberOfColours = sizeof(sc_coloursInUse) / sizeof(sc_coloursInUse[0]);
                    std::vector<cv::Rect> bricks{}; 

                    for (auto brickExmpl : brickExamples)
                    {
                        //get the location of our example bricks for backprojection and template matching
                        bricks = processor.findBrickLocations(brickExmpl, debugOrDemoAll);
                        if (bricks.empty()) continue;

                        //debugOrDemoAll = true;
                        std::sort(bricks.begin(), bricks.end(), [](const cv::Rect& left, const  cv::Rect& right)
                            {
                                return left.tl().x < right.tl().x;
                            });

                        if (bricks.size() == (numberOfColours + 1)) break; //Presumably found rectangles
                    }

                    if (bricks.size() != (numberOfColours + 1) || bricks.empty())
                    {
                        ErrorOutput(BrickCVErrors::EXAMPLE_BRICKS_ERROR, "Did not correctly identify", std::to_string(numberOfColours).c_str(), "colours in the exampleBricks image.");
                        continue; //this would scilently break the application, process next image instead
                    }

                    // Now we set up the colour detector with the scalars for our colours to get a better euclidian distance value
                    // The colour detector uses the bricks on top of the board to match the pixles on the image of the plate

                    // Alternatively predetermined shades defined through HMTL colour schemes can be used, though they only accurately determine 
                    // pixel colours in moderate lighting. Paricularly lighter colours or colours with multiple shades are prone to errors based on lighting here
                    auto detector = AbsColourDistance::ColourDetector();
                    std::map<BrickColour, cv::Rect> brickColourMap{};
                    std::map<BrickColour, cv::Scalar> brickScalarMap{};

                    int index = 0;
                    for (const auto brick : bricks)
                    {
                        auto expectedColour = sc_coloursInUse[index];
                        cv::Mat brickMat = exampleBricks(brick);
                        if (brickMat.empty() || brickMat.rows == 0 || brickMat.cols == 0)
                        {
                            Errors::ErrorOutput(Errors::NULL_PTR, "Image created from sample rectangle for sample bricks is empty or has dim * 0");
                            continue; 
                        }
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

                    debugOrDemoAll = true; 

                    detector.setBrickColourMap(brickScalarMap);

                    // now we create a "heatmap" of mapping the colours to the identified example bricks
                    // we do this by combining euclidean distance matchin in the LAB colour model, template matching the exmample brick 
                    // and also using a weighted addition of the LCH colour modle euclidean distance matching
                    // note: the last part was particularly useful to more reliably identify the following colours: Purple, Lime Green, Orange, Brown, Light Blue

                    std::map<BrickColour, std::shared_ptr<cv::Mat>> projections;
                    for (const auto [colourName, scalar] : brickScalarMap)
                    {
                        //make image of colour value and use it to identify pixels within threshold
                        auto meanColour = cv::Mat(30, 30, CV_8UC3, scalar);
                        auto projectRange = detector.findPixelsWithColourInRange(legoPlate, scalar, dynThres_LowerBoundry_LAB, dynThresh_UpperBoundry_LAB);

                        // make image of individual brick
                        auto rect = brickColourMap.at(colourName);
                        cv::Mat brickMat = exampleBricks(rect);

                        // use brick image and apply template matching - if lighting is very uneven this will fail 
                        cv::Mat minnimaMostLikely;
                        cv::matchTemplate(legoPlate, brickMat, minnimaMostLikely, TemplateMatchModes::TM_SQDIFF_NORMED);

                        // template matching creates a map with a global minnimum to represent the best match, so we invert it
                        cv::Mat resultMap = 1.0 - minnimaMostLikely;
                        resultMap.convertTo(resultMap, CV_8UC1, 255);
                        cv::medianBlur(resultMap, resultMap, 9);

                        // We repeat the process for the Chroma and Hue channel of the LCH colour model
                        auto lchHueChannel = detector.findPixelsWithColourInRangeForChannel(legoPlate, scalar, dynThres_LowerBoundry_LCH, dynThresh_UpperBoundry_LCH, ChannelType::LCHuv_HUE, false);
                        auto lchCromaChannel = detector.findPixelsWithColourInRangeForChannel(legoPlate, scalar, dynThres_LowerBoundry_LCH, dynThresh_UpperBoundry_LCH, ChannelType::LCHuv_CHROMA, false);
                        cv::Mat chromaHue;

                        if (debugOrDemoAll)
                        {
                            cv::imshow("LCH hue projection", lchHueChannel);
                            cv::imshow("LCH chroma projection", lchCromaChannel);

                            cv::waitKey(0);
                        }

                        // as a result of system testing it was decided to do a weighted addition 
                        // the chroma channel does provide valuable information regaring shades, but skews results if considered with the same ratio as the hue channel
                        // 0.3 was chosen after retesting with a batch of test images capturing lighting conditions outline in FR 1
                        if (colourName == BrickColour::YELLOW)
                        {
                            chromaHue = lchCromaChannel; 
                        }
                        else
                        {
                            cv::add(lchHueChannel, lchCromaChannel* weightFactor_ChromaChannel_LCH, chromaHue);
                        }

                        cv::medianBlur(chromaHue, chromaHue, 9);

                        if (debugOrDemoAll)
                        {
                            cv::imshow("LCH chroma", chromaHue);
                            cv::waitKey(0);
                        }

                        // ensure images are same size and add - shouldn't be needed, just done for sanity
                        cv::resize(lchHueChannel, lchHueChannel, cv::Size(legoPlate.cols, legoPlate.rows));
                        cv::resize(resultMap, resultMap, cv::Size(legoPlate.cols, legoPlate.rows));
                        cv::resize(projectRange, projectRange, cv::Size(legoPlate.cols, legoPlate.rows));

                        // The accuracy of the detection of brown & white rely heavily on the (otherwise) unprocessed channel in LCH
                        // so we limit the processing of that channel for efficency reasons 
                        if (colourName != BrickColour::WHITE && colourName != BrickColour::BROWN) cv::add(resultMap, chromaHue, resultMap);
                        cv::add(resultMap, projectRange, resultMap);

                        projections.emplace(colourName, std::make_shared<cv::Mat>(resultMap));

                        //In our heightmap we want rivers to be at the deepest point, so we invert the projection and add to the heightmap
                        if (colourName == BrickColour::DARK_BLUE)
                        {
                            cv::Mat deepRivers = 255 - resultMap;
                            processor.addToHeightMap(deepRivers);
                        }

                        if (debugOrDemoAll)
                        {
                            cv::Mat leg;
                            legoPlate.copyTo(leg);
                            cv::resize(leg, leg, cv::Size(300, 300));

                            cv::Mat res;
                            resultMap.copyTo(res);
                            cv::resize(res, res, cv::Size(300, 300));

                            cv::Mat col;
                            meanColour.copyTo(col);
                            cv::resize(col, col, cv::Size(100, 100));

                            cv::Mat brk;
                            brickMat.copyTo(brk);
                            cv::resize(col, col, cv::Size(100, 200));

                            const char* colour = getBrickColour(colourName);
                            string frameName = ("Avarage Colour shade: %s", colour);

                            cv::imshow(frameName.c_str(), res);
                            cv::imshow("Avarage colour scalar", col);
                            cv::imshow("Brick used for template matching", brk);
                            cv::imshow("Plate", leg);
                            cv::waitKey(0);
                        }
                    }

                    // debugOrDemoAll = true;

                    //------------------------------------------------------------------------
                    //generate output texture for biome mapping
                    for (auto [_, imgPtrs] : projections)
                    {
                        imgPtrs->reshape(0, legoPlate.rows);
                        auto img = imgPtrs.get();
                        resize(*img, *img, cv::Size(outRowsCols, outRowsCols));
                    }

                    auto base = projections.find(WHITE)->second;
                    cv::Mat outTexture = cv::Mat::zeros(base->rows, base->cols, CV_8UC3);
                    auto pRows = base->rows;
                    auto pCols = base->channels() * base->cols;

                    // TO DO: tidy up into seperate function
                    // TO DO: logging for projectioins.find(COLOR) - This breaks the application if second can't get accessed
                    // compare the result of out colour prediction heatmaps, and write the "most likely" value to the pixel 
                    for (auto rowIndex = 0; rowIndex < pRows; rowIndex++)
                    {
                        auto whiteProjectionRow = projections.find(WHITE)->second->ptr<uchar>(rowIndex);
                        auto purpleProjectionRow = projections.find(PURPLE)->second->ptr<uchar>(rowIndex);
                        auto dBlueProjectionRow = projections.find(DARK_BLUE)->second->ptr<uchar>(rowIndex);
                        auto lBlueProjectionRow = projections.find(LIGHT_BLUE)->second->ptr<uchar>(rowIndex);
                        auto brownProjectionRow = projections.find(BROWN)->second->ptr<uchar>(rowIndex);
                        auto redProjectionRow = projections.find(RED)->second->ptr<uchar>(rowIndex);
                        auto yellowProjectionRow = projections.find(YELLOW)->second->ptr<uchar>(rowIndex);
                        auto lGreenProjectionRow = projections.find(LIGHT_GREEN)->second->ptr<uchar>(rowIndex);

                        const uchar* pixelArr[8] = {
                            whiteProjectionRow
                            , purpleProjectionRow
                            , dBlueProjectionRow
                            , lBlueProjectionRow
                            , brownProjectionRow
                            , redProjectionRow
                            , yellowProjectionRow
                            , lGreenProjectionRow
                        };

                        for (auto columnIndex = 0; columnIndex < pCols; columnIndex++)
                        {
                            cv::Scalar pixelPredictionColour = BrickCV::getRGBScalar(DARK_GREEN);
                            uchar predictionValue = (uchar)200;
                            for (int colour = 0; colour < 8; colour++) //using the index to map to sc_coloursInUse
                            {
                                auto pixCol = sc_coloursInUse[colour];
                                auto colourRow = (pixelArr)[colour];
                                int pxVal = ((int)(colourRow[columnIndex]));
                                if (pxVal >= (int)predictionValue)
                                {
                                    predictionValue = colourRow[columnIndex];
                                    pixelPredictionColour = BrickCV::getRGBScalar(pixCol);
                                }
                            }

                            outTexture.at<cv::Vec3b>(rowIndex, columnIndex)[0] = (uchar)pixelPredictionColour[0];
                            outTexture.at<cv::Vec3b>(rowIndex, columnIndex)[1] = (uchar)pixelPredictionColour[1];
                            outTexture.at<cv::Vec3b>(rowIndex, columnIndex)[2] = (uchar)pixelPredictionColour[2];
                        }
                    }

                    if (debugOrDemoAll)
                    {
                        cv::Mat copyTexture;
                        outTexture.copyTo(copyTexture);
                        cv::resize(copyTexture, copyTexture, cv::Size(400, 400));
                        cv::imshow("Output texture Prediction", copyTexture);
                        cv::waitKey(0);
                    }

                    outTexture.convertTo(outTexture, CV_16FC3, 255.0); //Desired file format for heightmaps for UE5 editor landscape system - note that this was used in development but the end product uses a csv file to get the height data and produce a procedural mesh
                    cv::imwrite(textureOutputPath, outTexture);

                    //-------------------------------------------------------------------
                    // finish getting heightmap info
                    // we layer over infromation from the colour detection algorithm with some random noise
                    // then we reapeatedly resize and further process the heightmap until an "organic looking" result is achived
                    // similar to a classic perlin noise algorithm 
                    auto heightMap = processor.getHeightMap().lock().get();
                    cv::normalize(*heightMap, *heightMap, 0, 255, NORM_MINMAX);

                    cv::Mat dist = cv::Mat(heightMap->rows, heightMap->cols, CV_8UC1);
                    cv::randn(dist, 70, 30);
                    int zoom = dist.rows / 100;
                    cv::Rect distRect(0, 0, zoom, zoom);
                    cv::Mat noiseZoom = dist(distRect);
                    cv::resize(noiseZoom, noiseZoom, cv::Size(heightMap->cols, heightMap->rows));
                    auto slope = cv::Rect(0, 0, noiseZoom.cols, noiseZoom.rows);
                    cv::rectangle(noiseZoom, slope, cv::Scalar(50), 200);
                    cv::rectangle(noiseZoom, slope, cv::Scalar(0), 100);
                    cv::normalize(noiseZoom, noiseZoom, 0, 100, NORM_MINMAX);
                    cv::medianBlur(noiseZoom, noiseZoom, 25);
                    cv::add(*heightMap, noiseZoom, dist);
                    cv::medianBlur(dist, dist, 15);
                    cv::normalize(dist, dist, 0, 255, NORM_MINMAX);

                    //based on test results
                    //isolate big white blops in height map and blend them into the environment
                    cv::Mat tooHighAreas;
                    cv::threshold(dist, tooHighAreas, 230, 255, THRESH_BINARY);
                    cv::Mat noiseAdded;
                    noiseZoom.copyTo(noiseAdded, tooHighAreas);
                    cv::Mat out = dist * 0.5;
                    cv::add(out, noiseAdded, out);

                    cv::GaussianBlur(out, out, cv::Size(15, 15), 5, 5);
                    cv::normalize(out, out, 100, 250, NORM_MINMAX);
                    cv::resize(out, out, cv::Size(100, 100));

                    cv::GaussianBlur(out, out, cv::Size(15, 15), 50, 50);
                    cv::resize(out, out, cv::Size(heightMap->cols, heightMap->rows));
                    cv::normalize(out, out, 0, 255, NORM_MINMAX);
                    cv::medianBlur(out, out, 3);

                    //debugOrDemoAll = true; 
                    if (debugOrDemoAll) cv::resize(out, out, cv::Size(500, 500));
                    if (debugOrDemoAll) cv::imshow("HeightMap", out);
                    if (debugOrDemoAll) cv::waitKey(0);

                    //createHeightMapPNG = true; 
                    if (createHeightMapPNG) processor.createHeightMapPNG(out, heightMapOutputPath);

                    // In the final product we use the heightmap to create a 40 rows * 40 cols .csv file containing only the Z value of the relative
                    // coordinates of the landscape
                    int pixelNum = outRowsCols * outRowsCols;

                    cv::resize(out, out, cv::Size(outRowsCols, outRowsCols));
                    out.convertTo(out, CV_8UC1); //convert back to values between 0 - 255 stored as Uchars

                    out.reshape(0, outRowsCols); //sanity check - ensure image not continuous
                    auto pointData = std::vector<uint8_t>(pixelNum);
                    int vectorIndex = 0;

                    auto rows = out.rows;
                    auto cols = out.cols;

                    if (out.channels() == 1)
                    {
                        std::cout << "[Information] \t \t About to prepare point data. This will take a while..." << std::endl;
                        for (int16_t row = 0; row < rows; row++)
                        {
                            uchar* rowPtr = out.ptr<uchar>(row);
                            for (int16_t col = 0; col < cols; col++)
                            {
                                auto zVal = rowPtr[col];
                                auto& pointVal = pointData[vectorIndex];
                                uint8_t pixelValContainingZ = static_cast<uint8_t>(zVal);
                                pointVal = pixelValContainingZ;

                                vectorIndex++;
                            }
                        }
                    }

                    ofstream outCSV;
                    outCSV.open(outputfileName);
                    int pointCount = 0;
                    std::cout << "[Information] \t \t About to parse triangle data to .csv file. This will take a while..." << std::endl;

                    int linePlace = 1;
                    for (const uint8_t point : pointData)
                    {
                        // UE5s Fileloader and string class are very specific about the needed format
                        // Therefor we have to output the value as specifically int rather than char
                        auto castPoint = (int)point;
                        outCSV << castPoint;
                        if (linePlace < (outRowsCols)) outCSV << ",";
                        if (linePlace == (outRowsCols))
                        {
                            outCSV << std::endl;
                            linePlace = 0;
                        }
                        pointCount++;
                        linePlace++;
                    }
                    outCSV.close();
                    std::cout << "[INFORMATION] \t \t CSV File contains " << pointCount << " Z coordinates. Should be " << pixelNum << std::endl;

                    //TEST CSV reading
                    if (testCSV)
                    {
                        legoCVTests::CVAppOutputTester::testCSVOutput(outputfileName);
                    }
                    success = true; 
                    break;
                }
                if (success) break;
            }
            if (success) break;
        } 
        else
        {
            Errors::ErrorOutput(Errors::BrickCVErrors::NULL_PTR, "Image pointer could not be converted into a strong reference and had to be skipped");
        }
    } //END main img processing for loop

    std::cout << "PROCESSED IMAGES : " << imagesTried << " with last image name " << fileReader.m_fileNames[imagesTried-1] << std::endl; 

    //clean up images 
    fileReader.~ImageReader(); 

    std::cout << "--------------------------------------------------------------------------------------" << std::endl;
    std::cout << "---------------------------OPENCV LEGO MAP READER FINISHED----------------------------" << std::endl;
    std::cout << "--------------------------------------------------------------------------------------" << std::endl;
    return 0;
}
