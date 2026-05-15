#include "ImageProcessor.h"
#include "ImageProcessor.h"
#include "Histogram/SingleChannelHistogram.h"
#include "ERRORs/ErrorOutput.h"
#include "Histogram/ColourHistogram.h"
#include "Histogram/HistorgramGraphCreator.h"

//std includes
#include <iostream>
#include <memory>

//computer vision library incudes
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d.hpp>

void ImageProcessing::ImageProcessor::setWhiteBrick(cv::Mat& image)
{
    whiteBrick = std::make_shared<cv::Mat>(image);
}

cv::Mat ImageProcessing::ImageProcessor::boostValue(const cv::Mat& input, bool showResult)
{
    cv::Mat hsv;
    cv::Mat hsvChan[3];

    input.copyTo(hsv); 
    cv::cvtColor(hsv, hsv, cv::COLOR_BGR2HSV);
    cv::split(hsv, hsvChan);

    //hsvChan[1] *= 2; 
    hsvChan[2] *= 2;
    //cv::normalize(hsvChan[1], hsvChan[1], 0, 255, cv::NORM_MINMAX);
    cv::normalize(hsvChan[2], hsvChan[2], 0, 255, cv::NORM_MINMAX);
    cv::merge(hsvChan, 3, hsv);
    cv::cvtColor(hsv, hsv, cv::COLOR_HSV2BGR);

    if (showResult) cv::imshow("high saturation and value", hsv);
    //if (showResult) cv::imshow("Sat", hsvChan[1]);
    if (showResult) cv::imshow("Val", hsvChan[2]);
    cv::waitKey(0);

    return hsv; 
}

cv::Mat ImageProcessing::ImageProcessor::createRetinex(const cv::Mat& input, bool showResult)
{
    //based on openCV documentation : https://opencv.org/shadow-correction-using-opencv/
    cv::Mat shadowyInput;
    input.copyTo(shadowyInput);
    cv::cvtColor(shadowyInput, shadowyInput, cv::COLOR_BGR2Lab);
    shadowyInput.convertTo(shadowyInput, CV_32FC3); 

    cv::Mat labChannels[3]; 
    cv::split(shadowyInput, labChannels); 

    int scales[3] = {31, 101, 301};
    cv::Mat retinex = cv::Mat::zeros(input.rows, input.cols, CV_32FC1); 

    cv::Mat blurred; 
    for (auto scale : scales)
    {
        cv::GaussianBlur(labChannels[0], blurred, cv::Size(scale, scale), 0);
        cv::Mat logLuminanceP1 = labChannels[0] + 1; //avoid log(0)
        cv::Mat logblurredP1 = blurred + 1; 
        cv::log(logLuminanceP1, logLuminanceP1); 
        cv::log(logblurredP1, logblurredP1); 
        retinex += (logLuminanceP1 - logblurredP1); 
    }

    retinex /= 3; // size of scales array

    if (showResult) cv::imshow("Retinex L channel", retinex);
    if (showResult) cv::waitKey(0);

    return retinex; 
}

cv::Mat ImageProcessing::ImageProcessor::adaptiveShadowRemovalMask(const cv::Mat& input, const cv::Mat& retinex, double sensitivity /* = 1.0 */, bool showResult)
{
    // based on openCV documentation : https://opencv.org/shadow-correction-using-opencv/
    // combine the L channel in LAB with the Saturation in HSV to avoid creating new heus through skew 
    cv::Mat shadowyInput;
    cv::Mat shadowyHSV; 
    input.copyTo(shadowyInput);
    input.copyTo(shadowyHSV);
    cv::cvtColor(shadowyInput, shadowyInput, cv::COLOR_BGR2Lab);
    cv::cvtColor(shadowyHSV, shadowyHSV, cv::COLOR_BGR2HSV);
    shadowyInput.convertTo(shadowyInput, CV_32FC3);
    shadowyHSV.convertTo(shadowyHSV, CV_32FC3);

    cv::Mat lab[3];
    cv::Mat hsv[3];
    cv::split(shadowyInput, lab);
    cv::split(shadowyHSV, hsv);

    cv::Mat mask = cv::Mat::zeros(shadowyInput.rows, shadowyInput.cols, CV_32FC1);
    mask += (((lab[0] < 0.5) * sensitivity) & (hsv[1] < 0.5));

    if (showResult) cv::imshow("Adaptive shadow mask using LAB and HSV", mask);
    if (showResult) cv::waitKey(0);

    return mask; 
}

// custom function to replace the shadowy areas with pixels boosted in saturation and value. Also permanently applies the luminance transformation to the input image
cv::Mat ImageProcessing::ImageProcessor::removeShadows(const cv::Mat& input, cv::Mat& retinex, cv::Mat mask, bool showResult)
{
    if (mask.empty())
    {
        Errors::ErrorOutput(Errors::NULL_PTR, "Shadow mask is empty. Was the function called too early?");
        return cv::Mat();
    }

    // based on openCV documentation : https://opencv.org/shadow-correction-using-opencv/
    cv::Mat shadowyInput;
    input.copyTo(shadowyInput);
    cv::cvtColor(shadowyInput, shadowyInput, cv::COLOR_BGR2Lab);
    shadowyInput.convertTo(shadowyInput, CV_32FC3);

    cv::Mat labChannels[3];
    cv::split(shadowyInput, labChannels);

    auto elipsicalKernal = cv::getStructuringElement(cv::MorphShapes::MORPH_ELLIPSE, cv::Size(7,7)); 
    cv::Mat shadowMask; 
    cv::Mat shadowMask2; 
    cv::morphologyEx(mask, shadowMask, cv::MorphTypes::MORPH_CLOSE, elipsicalKernal); //close gaps
    cv::dilate(shadowMask, shadowMask2, elipsicalKernal, cv::Point(-1,-1), 1); 

    cv::medianBlur(shadowMask2, shadowMask2, 5);
    cv::pow(shadowMask2, 1.5 ,shadowMask2);

    if (showResult) cv::imshow("Shadow mask", shadowMask2);
    if (showResult) cv::waitKey(0);

    auto inverse = 1 - retinex; 
    cv::Mat Lchan = labChannels[0] + inverse;

    if (showResult) cv::imshow("L channel adjusted", Lchan);
    if (showResult) cv::waitKey(0);

    cv::Mat image[3] = {Lchan, labChannels[1], labChannels[2] };
    cv::Mat result; 
    cv::merge(image, 3, result);

    result.convertTo(result, CV_8UC3);
    cv::cvtColor(result, input, cv::COLOR_Lab2BGR);
    cv::cvtColor(result, result, cv::COLOR_Lab2BGR);

    if (showResult) cv::imshow("Evened out Luminance - result written to input image", result);
    if (showResult) cv::waitKey(0);

    shadowMask2.convertTo(shadowMask2, CV_8UC3);
    cv::resize(shadowMask2, shadowMask2, cv::Size(result.cols, result.rows));
    cv::Mat bgrShadows;
    result.copyTo(bgrShadows, shadowMask2);

    if (showResult) cv::imshow("BGR shadows", bgrShadows);
    if (showResult) cv::waitKey(0);

    cv::Mat hsvShadows;
    cv::cvtColor(bgrShadows, hsvShadows, cv::COLOR_BGR2HSV);
    cv::Mat hsv[3]; 
    cv::split(hsvShadows, hsv); 

    //bump hue and saturation channels
    hsv[1] += cv::saturate_cast<uchar>(20); 
    hsv[2] += cv::saturate_cast<uchar>(20);

    cv::Mat intense; 
    cv::merge(hsv, 3, intense); 
    cv::cvtColor(intense, intense, cv::COLOR_HSV2BGR);
    intense.copyTo(bgrShadows, shadowMask2);

    result += bgrShadows; 

    if (showResult) cv::imshow("BGR shadows 2", intense);
    if (showResult) cv::waitKey(0);

    if (showResult) cv::imshow("Removed shadows fin", result);
    if (showResult) cv::waitKey(0);

    return result; 
}

void ImageProcessing::ImageProcessor::addToHeightMap(const cv::Mat& info)
{
    cv::Mat infoReshaped;
    cv::resize(info, infoReshaped, cv::Size(sc_pixelsInHeightMap,sc_pixelsInHeightMap)); 
    if (m_heightMap == nullptr)
    {
        m_heightMap = std::make_shared<cv::Mat>(infoReshaped);
    }
    else
    {
        // we add the different layers of colours to create "noise" from the processing of the images so the output terrain 
        // is coherent with the designers vision and placing of the lego features
        cv::Mat added; 
        cv::add(*m_heightMap, infoReshaped, added);
        cv::normalize(added,added,0,255);
        m_heightMap.reset();
        m_heightMap = std::make_shared<cv::Mat>(added);
    }
}

void ImageProcessing::ImageProcessor::setImageOfLego(cv::Mat& image)
{
    imageOfLego = std::make_shared<cv::Mat>(image); 
}

void ImageProcessing::ImageProcessor::setImageOfBricks(cv::Mat& image)
{
    imageOfBricks = std::make_shared<cv::Mat>(image);
}

std::vector<cv::Rect> ImageProcessing::ImageProcessor::findBrickLocations(cv::Mat& brickArea, bool showResult /* = false */)
{
    auto demo = showResult; 
    cv::Mat brickAreaCopy; 
    if (showResult) brickArea.copyTo(brickAreaCopy); 

    cv::Mat brickChannels[3];
    cv::split(brickArea, brickChannels);
    cv::Mat brickMaskReddishColours;
    cv::Mat brickMaskBluishColours;

    brickMaskReddishColours = brickChannels[redOrBlueYellow] * 2; 
    brickMaskBluishColours = brickChannels[blueOrLuminance] * 2;

    threshold(brickMaskReddishColours, brickMaskReddishColours, 100, 255, cv::THRESH_BINARY);
    threshold(brickMaskBluishColours, brickMaskBluishColours, 100, 255, cv::THRESH_BINARY);

    cv::medianBlur(brickMaskReddishColours, brickMaskReddishColours, 5); 
    cv::medianBlur(brickMaskBluishColours, brickMaskBluishColours, 5);

    auto elipsicalKernal = cv::getStructuringElement(cv::MorphShapes::MORPH_ELLIPSE, cv::Size(7, 7));
    cv::morphologyEx(brickMaskReddishColours, brickMaskReddishColours, cv::MorphTypes::MORPH_CLOSE, elipsicalKernal); //close gaps
    cv::morphologyEx(brickMaskBluishColours, brickMaskBluishColours, cv::MorphTypes::MORPH_CLOSE, elipsicalKernal); //close gaps

    if (demo) cv::imshow("Brick mask Reds", brickMaskReddishColours);
    if (demo) cv::imshow("Brick mask Blues", brickMaskBluishColours);
    if (demo) cv::waitKey(0);

    this->getContourData(brickMaskReddishColours, false);
    auto rectanglesPtrReds = this->getRectangles();

    this->getContourData(brickMaskBluishColours, false);
    auto rectanglesPtrBlues = this->getRectangles();

    std::shared_ptr<std::vector<cv::Rect>> presumedBricks[2];
    presumedBricks[0] = rectanglesPtrReds;
    presumedBricks[1] = rectanglesPtrBlues;

    //if (demo)  cv::imshow("MSER Mask for thresholding regions", mserMask);
    auto bricks = std::vector<cv::Rect>();

    //this->getContourData(mserMask, true);
    //auto brickRectangles = this->getRectangles(); 
    //presumedBricks[0] = brickRectangles; 

    for (auto rect : presumedBricks)
    {
        if (rect != nullptr && rect->size() != 0)
        {
            for (auto rectangleInstance : *rect)
            {
                bool isBrickShaped = (rectangleInstance.height >= rectangleInstance.width * 1.9);
                if ((((rectangleInstance.width + rectangleInstance.height) != 0) && isBrickShaped))
                {
                    //shrink the rectangle to exclude unwanted pixls
                    auto smallerRectTLX = rectangleInstance.tl().x + (rectangleInstance.width *0.8);
                    auto smallerRectTLY = rectangleInstance.tl().y + (rectangleInstance.height *0.8);
                    auto smallerRectBRX = rectangleInstance.br().x - (rectangleInstance.width *0.8);
                    auto smallerRectBRY = rectangleInstance.br().y - (rectangleInstance.height *0.8);

                    cv::Rect smallerRect(cv::Point(static_cast<int>(smallerRectTLX), static_cast<int>(smallerRectTLY)), cv::Point(static_cast<int>(smallerRectBRX), static_cast<int>(smallerRectBRY)));

                    if (showResult) cv::rectangle(brickAreaCopy, rectangleInstance, CV_RGB(255,0,255), 3);
                    if (showResult) cv::rectangle(brickAreaCopy, smallerRect, CV_RGB(0, 255, 0), 3);

                    bool dontPlace = false;
                    bool isInBricks = false;
                    for (auto brickRectangle : bricks)
                    {
                        // Check if this rectangle overlaps with any of our previously selected areas
                        // for synatx explanation refer to https://putuyuwono.wordpress.com/2015/06/26/intersection-and-union-two-rectangles-opencv/
                        isInBricks = (smallerRect & brickRectangle).area() > 0;
                        if (isInBricks) dontPlace = true;
                    }
                    if (!dontPlace)
                    {
                        bricks.emplace_back(smallerRect);
                    }
                    else if(showResult) cv::rectangle(brickAreaCopy, smallerRect, CV_RGB(255, 0, 0), 3); //should mark white ROI in red
                }
            }
        }
    }

    if (showResult)
    {
        cv::imshow("Brick area rectangles", brickAreaCopy);
        cv::waitKey(0); 
    }

    return bricks; 
}

// Uses cv::Mat reference to resize to sc_pixelsHeightMap^2 and then converts to Uint 16 type image, then outputs "m_heightMap.png" and returns cv::Mat if needed.
cv::Mat ImageProcessing::ImageProcessor::createHeightMapPNG(cv::Mat& heightMapData, const char* heightMapOutPutPath)
{
    cv::resize(heightMapData, heightMapData, cv::Size(sc_pixelsInHeightMap, sc_pixelsInHeightMap));
    heightMapData.convertTo(heightMapData, CV_16U, 255.0); //Desired file format for heightmaps for UE5 editor landscape system - note that this was used in development but the end product uses a csv file to get the height data and produce a procedural mesh
    cv::imwrite(heightMapOutPutPath, heightMapData);
    return heightMapData;
}

void ImageProcessing::ImageProcessor::getContourData(cv::Mat& allEdgesAdded, bool drawContours)
{
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(allEdgesAdded, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
    std::vector<cv::Rect> boundRect(contours.size());

    for (size_t i = 0; i < contours.size(); i++)
    {
        cv::approxPolyDP(contours[i], contours[i], 4, true);
        boundRect[i] = boundingRect(contours[i]);
    }

    if (drawContours)
    {
        drawenContours.release();
        drawenContours = cv::Mat(allEdgesAdded.cols, allEdgesAdded.rows, CV_8UC1);
        cv::drawContours(drawenContours, contours, -1, CV_RGB(255,255,255)); 
    }

    this->m_boundRect = std::make_shared<std::vector<cv::Rect>>(boundRect);
    this->m_contours = std::make_shared<std::vector<std::vector<cv::Point>>>(contours);
}

cv::Rect ImageProcessing::ImageProcessor::findRectWithLongestSide(const std::vector<std::vector<cv::Point>>& contours, cv::Rect& topleftGreenCorner) {
    cv::Rect longestSideRect;
    int maxSide = 0;

    for (const auto& contour : contours) {
        cv::Rect rect = cv::boundingRect(contour);
        if ((static_cast<int>(rect.tl().y) > (static_cast<int>(topleftGreenCorner.tl().y)+10) && (static_cast<int>(rect.height) < static_cast<int>(topleftGreenCorner.height))))
        {
            int currentMaxSide = rect.width;

            if (currentMaxSide > maxSide) {
                maxSide = currentMaxSide;
                longestSideRect = rect;
            }
        }
    }
    return longestSideRect;
}


cv::Rect ImageProcessing::ImageProcessor::findRectWithLargestVolium(const std::vector<cv::Rect>& boxes) {
    cv::Rect greatestVolium;
    int maxVol = 0;

    for (const auto& box : boxes) 
    {
        int currentMaxVol = box.area();

        if (currentMaxVol > maxVol) 
        {
            maxVol =  currentMaxVol;
            greatestVolium = box;
        }
    }
    return greatestVolium;
}

//returns 0 area Rect if no bounding box is roughly square
cv::Rect ImageProcessing::ImageProcessor::findLargestVoliumSquareContour(std::vector<std::vector<cv::Point>>& boxes) {
    auto greatestVolium = cv::Rect(0,0,0,0);
    int maxVolium = 0; 

    for (const auto box : boxes) 
    {
        auto square = findSquareIsh(box); 
        if (square.width*square.height > maxVolium)
        {
            greatestVolium = square;
            maxVolium = square.width * square.height;
        }
    }
    return greatestVolium;
}

//returns 0 area Rect if not squarish
cv::Rect ImageProcessing::ImageProcessor::findSquareIsh(const std::vector<cv::Point>& box)
{
    auto rect = cv::boundingRect(box);
    double aspectRatio = static_cast<double>(rect.width) / static_cast<double>(rect.height);
    bool isInRatio = aspectRatio >= 0.8 && aspectRatio <= 1.2; 
    if (isInRatio)
    {
       return rect;
    }
    return cv::Rect(0,0,0,0); 
}


// This relies haevily on how cv::MSER::create is set up vs how the light conditions are, which is why it was phased out
cv::Mat ImageProcessing::ImageProcessor::getMSERMask(cv::Mat unblurredImage, bool showAreas /* = false */)
{
    cv::Mat mserMask; 
    mserMask.create(unblurredImage.rows, unblurredImage.cols, CV_8UC1);
    cv::Ptr<cv::MSER> mserAlgorithm = cv::MSER::create(5, 9, ((unblurredImage.cols*unblurredImage.rows)/4),0.28,0.01, 500, 1.01, 0.003, 9);

    std::vector<std::vector<cv::Point> > mserContours;
    std::vector<cv::Rect> mserRegions;
    mserAlgorithm->detectRegions(unblurredImage, mserContours, mserRegions);

    uchar colourValue = 0; 
    for (std::vector<cv::Point> pointVector : mserContours) 
    {
        for (cv::Point point : pointVector) {
            mserMask.at<uchar>(point.y, point.x) = colourValue;
        }
        colourValue += showAreas ? 5 : 0;
    }

    this->mserMask = std::make_shared<cv::Mat>(mserMask); 

    if (showAreas) cv::imshow("Maximally stable extremal regions - greyscale (up to 25 regions shown)", mserMask);
    if (showAreas) cv::waitKey(0);

    return mserMask; 
}

// custom way of adding the contours and colour channel to create a map 
cv::Mat ImageProcessing::ImageProcessor::createMapWithContoursAndLABchannel(const cv::Mat& labChannel, const cv::Mat& mser)
{
    if (labChannel.rows != mser.rows || labChannel.cols != mser.cols)
    {
        Errors::ErrorOutput(Errors::IMAGEs_NOT_EQUAL_SIZE, "Input for this funciton has to have equal rows and columns but didn't."); 
        return cv::Mat(); 
    }

    cv::Mat outMask = cv::Mat::zeros(labChannel.rows, labChannel.cols, CV_8UC1); 

    auto rows = labChannel.rows;
    auto cols = labChannel.channels() * labChannel.cols;

    auto bothContinuous = labChannel.isContinuous() && mser.isContinuous(); 
    if (bothContinuous)
    {
        cols = cols * rows;
        rows = 1;
    }

    //loop over all pixels in the images
    for (auto rowIndex = 0; rowIndex < rows; rowIndex++)
    {
        auto labPx = labChannel.ptr<uchar>(rowIndex);
        auto mserPx = mser.ptr<uchar>(rowIndex);
        auto outPx = outMask.ptr<uchar>(rowIndex); 

        for (auto columnIndex = 0; columnIndex < cols; columnIndex++)
        {
            //if the pixel is black in the mser Mask we keep it black, otherwise we add the LAB channel Value
            if (mserPx[columnIndex] != 0)
            {
                outPx[columnIndex] = labPx[columnIndex]; 
            } 
            else
            {
                outPx[columnIndex] = 0;
            }
        }
    }
    return outMask; 
}

cv::Rect ImageProcessing::ImageProcessor::findLegoWithThresholdingMask(cv::Mat imageToProcess, int lowerboundGreen, bool showGreenMask /* = false */, bool showAllRect /* = false */) {

    cv::Mat legoPixMask;
    cv::Mat copy; 
    imageToProcess.copyTo(copy); 

    cv::threshold(copy, legoPixMask, 50, 255, cv::THRESH_BINARY);
    cv::cvtColor(legoPixMask, legoPixMask, cv::COLOR_BGR2GRAY);
    cv::threshold(legoPixMask, legoPixMask, 100, 255, cv::THRESH_BINARY);
    cv::medianBlur(legoPixMask, legoPixMask, 9); 

    this->legoPXMask = std::make_shared<cv::Mat>(legoPixMask); 
    
    if (showGreenMask)
    {
        cv::imshow("ROI on black tray", legoPixMask);
        cv::waitKey(0);
    }

    //find contours in thresholded
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hirarchy;
    cv::findContours(legoPixMask, contours, hirarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
    std::vector<std::vector<cv::Point>> rectangleCont(contours.size());
    std::vector<cv::Rect> boundRect(contours.size());

    for (size_t i = 0; i < contours.size(); i++)
    {
        approxPolyDP(contours[i], rectangleCont[i], 4, true);
        boundRect[i] = cv::boundingRect(rectangleCont[i]);
        if (showAllRect)
        {
            cv::rectangle(imageToProcess, boundRect[i], CV_RGB(255, 0, 0), 2); 
        }
    }

    auto presumedPlate = this->findLargestVoliumSquareContour(contours); 

    copy.release(); 

    return presumedPlate;
}

// The following function is based on https://stackoverflow.com/questions/54948836/findcontours-and-retr-tree-iterate-through-hierarchy
// Theory - find contours RETR_TREE outputs a hirarchy. We want to find the inside elements on the black tray. Therefor we are looking for
//         the largest square shaped contour on level 1 with children
// For more informations on how hirarchy is structured, please visit "https://docs.opencv.org/3.4/d9/d8b/tutorial_py_contours_hierarchy.html"
cv::Rect ImageProcessing::ImageProcessor::findLargestVoliumChild(std::vector<cv::Vec4i> hierarchy, std::vector<std::vector<cv::Point>> contours, cv::Mat& roi, bool showResult)
{
    auto outRect = cv::Rect(0, 0, 0, 0);

    for (int parentIdx(0); parentIdx >= 0; parentIdx = hierarchy[parentIdx][NEXT_SIBLING]) 
    {
        //Not interrested in contours with no children
        if (hierarchy[parentIdx][CHILD_CONTOUR] == -1) {
            if (showResult)
            {
                auto drawRect = cv::boundingRect(contours[parentIdx]); 
                cv::rectangle(roi, drawRect, cv::Scalar(255, 0, 0));
            }
            continue;
        }

        if (showResult)
        {
            int contourIDx = hierarchy[parentIdx][CHILD_CONTOUR];
            auto drawRect = cv::boundingRect(contours[contourIDx]);
            cv::rectangle(roi, drawRect, cv::Scalar(0, 0,255));
        }

        // Iterate over all direct children and find largest
        int maxVol = 0; 
        for (int childIDx(hierarchy[parentIdx][CHILD_CONTOUR]); childIDx >= 0; childIDx = hierarchy[childIDx][NEXT_SIBLING]) {

            if (showResult)
            {
                auto drawRect = cv::boundingRect(contours[childIDx]);
                cv::rectangle(roi, drawRect, cv::Scalar(0, 255, 0));
            }

            cv::Rect square = findSquareIsh(contours[childIDx]);
            if (square.width*square.height > maxVol) 
            {
                //We found a square-ish with children, so hopefully the plate
                outRect =  cv::boundingRect(contours[childIDx]); 
                maxVol = square.width * square.height;
            }
        }

        if (showResult)
        {
            cv::rectangle(roi, outRect, CV_RGB(50, 100, 150), 2);
            cv::imshow("Squareish shape found in black area", roi);
            cv::waitKey(0);
        }
        return outRect;
    }
    return outRect;
}

cv::Rect ImageProcessing::ImageProcessor::findChildCorners(int largestVoliumIndex, const std::vector<cv::Rect>& boxes, std::vector<cv::Vec4i> hierarchy, std::vector<std::vector<cv::Point>> contours, cv::Mat& preProcessedGrey, bool showResult)
{
    auto outRect = cv::Rect(0, 0, 0, 0);
    for (int parentIdx(0); parentIdx >= 0; parentIdx = hierarchy[parentIdx][NEXT_SIBLING])
    {
        //Not interrested in contours with no children
        if (hierarchy[parentIdx][CHILD_CONTOUR] == -1) {
            if (showResult)
            {
                auto drawRect = cv::boundingRect(contours[parentIdx]);
                cv::rectangle(preProcessedGrey, drawRect, cv::Scalar(255, 0, 0));
            }
            continue;
        }

        if (showResult)
        {
            int contourIDx = hierarchy[parentIdx][CHILD_CONTOUR];
            auto drawRect = cv::boundingRect(contours[contourIDx]);
            cv::rectangle(preProcessedGrey, drawRect, cv::Scalar(0, 0, 255));
        } 

        int childcontours = 0;
        std::vector<std::vector<cv::Point>> childrenWith6Points;
        for (int childIDx(hierarchy[parentIdx][CHILD_CONTOUR]); childIDx >= 0; childIDx = hierarchy[childIDx][NEXT_SIBLING]) 
        {
            //not interested in any non corner contours with less than 6 points
            childrenWith6Points.emplace_back(contours[childIDx]);
            childcontours++;
        }

        if (showResult)
        {
            for (auto cont : childrenWith6Points)
            {
                cv::rectangle(preProcessedGrey, cv::boundingRect(cont), CV_RGB(50, 100, 150), 3);
            }
            cv::imshow("Child boxes internal found", preProcessedGrey);
            cv::waitKey(0);
        }
    }
    return outRect;
}

// DEPRECATED: was written for locating white bricks and using that information. No longer part of the core logic
void ImageProcessing::ImageProcessor::setXCoordinatesForWhiteBricks(std::vector<cv::Vec4i> hierarchy, std::vector<std::vector<cv::Point>> contours, cv::Mat& roi, bool showResult)
{
    auto outRect = cv::Rect(0, 0, 0, 0);

    for (int parentIdx(0); parentIdx >= 0; parentIdx = hierarchy[parentIdx][NEXT_SIBLING])
    {
        //Not interrested in contours with no children - we want to find the black tray which would have children based previous thresholding
        if (hierarchy[parentIdx][CHILD_CONTOUR] == -1) {
            if (showResult)
            {
                auto drawRect = cv::boundingRect(contours[parentIdx]);
                cv::rectangle(roi, drawRect, cv::Scalar(255, 0, 0));
            }
            continue;
        }

        // Iterate over all direct children and coordinates of top corners
        // We want right most and left most corners while maintaining roughly the same height
        int brickHeight = 0; 
        int xLeftmost = 9999999;
        int xRightMost = 0; 
        for (int childIDx(hierarchy[parentIdx][CHILD_CONTOUR]); childIDx >= 0; childIDx = hierarchy[childIDx][NEXT_SIBLING]) 
        {
            auto rect = boundingRect(contours[childIDx]); 
            int topLeftCorner = rect.x;
            int topRightCorner = rect.x + rect.width;

            if (brickHeight < (rect.y - rect.height*0.75)) //check if y coordinate is high enough in frame
            {
                // previous bounding box is more than two thirds the height of a brick below current target
                // because bricks should be the "highest" children we update our assumption 
                brickHeight = rect.y; 
                xLeftmost = topLeftCorner; 
                xRightMost = topRightCorner;
                continue;
            } 

            if (topLeftCorner < xLeftmost) xLeftmost = topLeftCorner;
            if (topRightCorner > xRightMost) xRightMost = topRightCorner;
        }
        this->rightWhiteMarkerTR_X = xRightMost; 
        this->leftWhiteMarkerTL_X =xLeftmost; 
    }
}


/*
* This function is large, but simple
* We take all of the contours, find the biggest one, and **ASSUME** it roughly covers the lego plate if we create a bounding rectangle around it.
* We then create a center point, and use basic trigonomitry to find the best match for corners of the lego plate 
*/
std::vector<cv::Point2f> ImageProcessing::ImageProcessor::useContoursToFindCorners(cv::Mat original, cv::Mat& greyScaleGreenChannel, bool showBlackMask /* = false */, bool showAllRect /* = false */) 
{
    cv::Mat orig;
    if (showAllRect)
    {
        original.copyTo(orig);
    }

    cv::Mat blackTrayMask = greyScaleGreenChannel;
    blackTrayMask = this->applySobel(blackTrayMask);

    if (showBlackMask)
    {
        cv::imshow("Sobel contours", blackTrayMask);
        cv::waitKey(0);
    }

    // find contours 
    // the largest contour should be our lego plate
    // it's bounding rectangle at least should enable us to find any missing points 
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(blackTrayMask, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
    std::vector<cv::Rect> boundRect(contours.size());

    double maxContourVolium = 0;
    int maxContourID = 0;
    int colour = 0;
    for (int contourIndex = 0; contourIndex < contours.size(); contourIndex++)
    {
        if (showAllRect)
        {
            cv::drawContours(orig, contours, contourIndex, CV_RGB(50, colour, 255-colour), 3);
            colour = (colour < 230 ? colour + 10 : 255);
        }

        boundRect[contourIndex] = cv::boundingRect(contours[contourIndex]);
        double newArea = cv::contourArea(contours.at(contourIndex));
        if (newArea > maxContourVolium)
        {
            maxContourID = contourIndex;
            maxContourVolium = newArea;
        }
    }

    if (contours.size() == 0)
    {
        Errors::ErrorOutput(Errors::BrickCVErrors::NO_CONTOURS_FOUND, "An image we were trying to process has no contours found.");
        if (showAllRect || showBlackMask) cv::imshow("No contours findable for this", orig); 
        cv::waitKey(0); 
        return std::vector<cv::Point2f>(); 
    }

    // create point in middle. If we debug show this frame, it will be displayed in a reddish shade
    auto largest  = cv::boundingRect(contours[maxContourID]);
    m_biggestRect = std::make_shared<cv::Rect>(largest);
    cv::Point2f centerOfLargest(static_cast<float>((largest.tl().x + largest.width / 2)), static_cast<float>((largest.tl().y + largest.height / 2)));

    if (showAllRect)
    {
        cv::circle(orig, centerOfLargest, 6, CV_RGB(100, 20, 50), cv::FILLED);
        cv::drawContours(orig, contours, maxContourID, CV_RGB(200, 50, 200), 3);
        cv::rectangle(orig, largest, CV_RGB(150, 200, 0), 3);
    }

    auto circumfrance = cv::arcLength(contours[maxContourID], true);
    std::vector<cv::Point2f> largestCorners;
    cv::approxPolyDP(contours[maxContourID], largestCorners, circumfrance * 0.075, true); //epsilon is generous

    if (largestCorners.size() <= 4)
    {
        // we use the center of the bounding rectangle to see what points we have and which are missing
        // we order them top left, top right, bottom right, bottom left as needed by the openCV function that creates the warp matrix
        cv::Point2f orderdPoints[4];
        bool foundTopLeft = false;
        bool foundBotLeft = false;
        bool foundTopRight = false;
        bool foundBotRight = false;

        // if we have less than 4 points we need to find the missing points
        // once all are found we emplace them according to the order we require for the warp matrix 
        if (!foundTopLeft)  orderdPoints[0] = findTL(contours, centerOfLargest);
        if (!foundTopRight) orderdPoints[1] = findTR(contours, centerOfLargest);
        if (!foundBotRight) orderdPoints[2] = findBR(contours, centerOfLargest);
        if (!foundBotLeft)  orderdPoints[3] = findBL(contours, centerOfLargest);

        largestCorners.clear();
        largestCorners.emplace_back(orderdPoints[0]);
        largestCorners.emplace_back(orderdPoints[1]);
        largestCorners.emplace_back(orderdPoints[2]);
        largestCorners.emplace_back(orderdPoints[3]);

        if (showAllRect)
        {
            auto colour = 0;
            for (auto point : largestCorners)
            {
                // colours clockwise from TL to BR go from yellow to pink
                cv::circle(orig, point, 5, CV_RGB(255, 200 - colour, colour), cv::FILLED);
                colour += 45;
            }
            cv::imshow("Points detected", orig);
            cv::waitKey(0);
        }
        return largestCorners;
    }
    else
    {
        if (showAllRect)
        {
            cv::imshow("More than 4 points detected, this will fail", orig);
            cv::waitKey(0);
        }
    }
    return std::vector<cv::Point2f>();
}

cv::Point2f ImageProcessing::ImageProcessor::findTL(std::vector<std::vector<cv::Point>> contours, cv::Point middle)
{
    //x has to be smaller and y has to be smaller, and distance to middle maximised
    double distanceSquared = 0; 
    cv::Point out(0,0); 
    for (auto cont : contours)
    {
        for (auto point : cont)
        {
            int tollerance = 5; 
            bool isInLargestRect = (point.x > m_biggestRect->tl().x - tollerance && point.x < m_biggestRect->br().x + tollerance) && (point.y > m_biggestRect->tl().y - tollerance && point.y < m_biggestRect->br().y + tollerance);
            if (isInLargestRect && point.x < middle.x && point.y < middle.y)
            {
                double pointDistanceSquared = ((middle.x - point.x) ^ 2) + ((middle.y - point.y) ^ 2); 
                if (pointDistanceSquared > distanceSquared)
                {
                    out = cv::Point2f(point);
                    distanceSquared = pointDistanceSquared; 
                }
            }
        }
    }
    return out; 
}

cv::Point2f ImageProcessing::ImageProcessor::findTR(std::vector<std::vector<cv::Point>> contours, cv::Point middle)
{
    //x has to be bigger and y has to be smaller, and distance to middle maximised
    double distanceSquared = 0;
    cv::Point out(0, 0);
    for (auto cont : contours)
    {
        for (auto point : cont)
        {
            int tollerance = 5;
            bool isInLargestRect = (point.x > m_biggestRect->tl().x - tollerance && point.x < m_biggestRect->br().x + tollerance) && (point.y > m_biggestRect->tl().y - tollerance && point.y < m_biggestRect->br().y + tollerance);
            if (isInLargestRect && point.x > middle.x && point.y < middle.y)
            {
                double pointDistanceSquared = ((point.x-middle.x) ^ 2) + ((middle.y - point.y) ^ 2);
                if (pointDistanceSquared > distanceSquared)
                {
                    out = cv::Point2f(point);
                    distanceSquared = pointDistanceSquared;
                }
            }
        }
    }
    return out;
}

cv::Point2f ImageProcessing::ImageProcessor::findBL(std::vector<std::vector<cv::Point>> contours, cv::Point middle)
{
    //x has to be smaller and y has to be bigger, and distance to middle maximised
    double distanceSquared = 0;
    cv::Point out(0, 0);
    for (auto cont : contours)
    {
        for (auto point : cont)
        {
            int tollerance = 5;
            bool isInLargestRect = (point.x > m_biggestRect->tl().x - tollerance && point.x < m_biggestRect->br().x + tollerance) && (point.y > m_biggestRect->tl().y - tollerance && point.y < m_biggestRect->br().y + tollerance);
            if (isInLargestRect && point.x < middle.x && point.y > middle.y)
            {
                double pointDistanceSquared = ((middle.x - point.x) ^ 2) + ((point.y- middle.y) ^ 2);
                if (pointDistanceSquared > distanceSquared)
                {
                    out = cv::Point2f(point);
                    distanceSquared = pointDistanceSquared;
                }
            }
        }
    }
    return out;
}

cv::Point2f ImageProcessing::ImageProcessor::findBR(std::vector<std::vector<cv::Point>> contours, cv::Point middle)
{
    //x has to be bigger and y has to be bigger, and distance to middle maximised
    double distanceSquared = 0;
    cv::Point out(0, 0);
    for (auto cont : contours)
    {
        for (auto point : cont)
        {
            int tollerance = 5;
            bool isInLargestRect = (point.x > (m_biggestRect->tl().x - tollerance) && point.x <= (m_biggestRect->br().x + tollerance)) && (point.y > (m_biggestRect->tl().y - tollerance) && point.y < (m_biggestRect->br().y + tollerance));
            if (isInLargestRect && point.x > middle.x && point.y > middle.y)
            {
                double pointDistanceSquared = ((point.x - middle.x) ^ 2) + ((point.y - middle.y) ^ 2);
                if (pointDistanceSquared > distanceSquared)
                {
                    out = cv::Point2f(point);
                    distanceSquared = pointDistanceSquared;
                }
            }
        }
    }
    return out;
}

 /*
 * This function capitalises of the base plate being green and still works well in low light with harsh shadows
 * If it fails we want to request another image from the camera
 */
cv::Rect ImageProcessing::ImageProcessor::getPlateWithGreenChannel(cv::Mat unprocessedROI, bool showResult)
{
    cv::Mat channels[3];
    split(unprocessedROI, channels);
    cv::Mat greenThresh;
    medianBlur(channels[1], channels[1], 9);
    cv::threshold(channels[1], greenThresh, 50, 255, cv::THRESH_BINARY);
    auto edges = this->applySobel(greenThresh);

    if (showResult)
    {
        cv::imshow("Green channel thresholded", greenThresh);
        cv::imshow("Green channel edges", edges);
        cv::waitKey(0);
    }

    this->getContourData(edges, false);
    auto contourPoints = this->getContourPoints();
    auto locked = contourPoints.lock(); 
    auto largest = this->findLargestVoliumSquareContour(*locked);

    if (showResult)
    {
        cv::Mat imagecopy; 
        unprocessedROI.copyTo(imagecopy);
        for (auto contour : *locked)
        {
            auto rec = cv::boundingRect(contour);
            cv::rectangle(imagecopy, rec, CV_RGB(50, 200, 150),2);
        }
        imshow("Fallback rectangles", imagecopy);

        rectangle(unprocessedROI, largest, CV_RGB(255, 255, 0), 2);
        imshow("[Initial detection failed, so fallback triggered] Fallback Output", unprocessedROI);
        cv::waitKey(0);
    }
    return largest; 
}

//TO DO: check y values
bool ImageProcessing::ImageProcessor::isWithinTollerance(cv::Rect& output)
{
    // check if result width is within reasonable distance to right most and left most corners
    // we know:
    // - a plate has 32 studs
    // - camera angle can add discrepancy 
    // we accept the distance to be off by up to 4 studs worth of space at most
    if (leftWhiteMarkerTL_X != -1 && rightWhiteMarkerTR_X != -1)
    {
        int distance = rightWhiteMarkerTR_X - leftWhiteMarkerTL_X;
        int studSize = distance / STUDS_PER_PLATE;
        int acceptedDivergance = 4 * studSize;
        if (output.x > leftWhiteMarkerTL_X + acceptedDivergance || output.x < leftWhiteMarkerTL_X - acceptedDivergance)
        {
            Errors::ErrorOutput(Errors::BrickCVErrors::COORDINATE_MISMATCH, "Tollerance of roughly 4 studs. Top Left X was out of bounds");
            return false;
        }

        if (output.br().x > rightWhiteMarkerTR_X + acceptedDivergance || output.br().x < rightWhiteMarkerTR_X - acceptedDivergance)
        {
            Errors::ErrorOutput(Errors::BrickCVErrors::COORDINATE_MISMATCH, "Tollerance of roughly 4 studs. Bottom Right X was outside bounds");
            return false;
        }
        return true; 
    }
    else
    {
        Errors::ErrorOutput(Errors::BrickCVErrors::FUNCTION_CALLED_TOO_SOON, "If this is called before the X values for the white bricks are set this will return an invalid result and produce undefined behavious.");
        return false; 
    }
}

void ImageProcessing::ImageProcessor::setContouringThresholds(cv::Mat& blurredGreyscale)
{
    normalize(blurredGreyscale, blurredGreyscale, 0, 255, cv::NORM_MINMAX);
    cv::minMaxIdx(blurredGreyscale, &controurThreshMin, &controurThreshMax, 0, 0);
    controurThreshMiddle = (controurThreshMin + controurThreshMax) / 2;
    double maxThreshCan = controurThreshMiddle * 1.5;
    if (maxThreshCan > 255)
        maxThreshCan = 255;
    double minThreshCan = maxThreshCan * 0.9;
}

cv::Mat ImageProcessing::ImageProcessor::applyCannyToBGR(cv::Mat& blurredBGR)
{
    cv::Mat blurredGreyscale; 
    cv::cvtColor(blurredBGR, blurredGreyscale, cv::COLOR_BGR2GRAY); 
    setContouringThresholds(blurredGreyscale);
    cv::Canny(blurredGreyscale, blurredGreyscale, controurThreshMin, controurThreshMax);
    return blurredGreyscale; 
}

cv::Mat ImageProcessing::ImageProcessor::applyCannyTo1D(cv::Mat& blurredGrey, int threshold)
{
    setContouringThresholds(blurredGrey);
    cv::Canny(blurredGrey, blurredGrey, threshold, controurThreshMax);
    return blurredGrey;
}

cv::Mat ImageProcessing::ImageProcessor::applySobel(cv::Mat& mask, int k)
{
    cv::Mat sobelx, sobely, gradient;
    cv::Sobel(mask, sobelx, CV_64F, 1, 0, k);
    cv::Sobel(mask, sobely, CV_64F, 0, 1, k);
    cv::magnitude(sobelx, sobely, gradient);

    // Convert to 8-bit image
    cv::convertScaleAbs(gradient, gradient);
    return gradient;
}

cv::Mat ImageProcessing::ImageProcessor::backprojectHistogram(cv::Mat& inputImage, cv::Mat& regionOfInterest, int threshold)
{
    //convert input image and ROI to lab and split channels
    //we seperate out the A & B channel
    cv::Mat labImage;
    cv::Mat labROI;
    cv::cvtColor(inputImage, labImage, cv::COLOR_BGR2Lab);
    cv::cvtColor(regionOfInterest, labROI, cv::COLOR_BGR2Lab);
    cv::Mat colourMatch; //outputmask
    colourMatch.create(labImage.size(), labImage.depth());

    cv::Mat labImageCh[3]; 
    cv::split(labImage, labImageCh); 
    cv::Mat labROIch[3];
    cv::split(labROI, labROIch);
    cv::Mat* axisCh = &labImageCh[1];
    cv::Mat* byCh = &labImageCh[2];

    cv::waitKey(0);

    auto axisHist = Histogram::ColourHistogram(BrickCV::BrickColour::RED, BrickCV::ChannelType::LAB_AXIS, labROIch[1]); 
    auto blueYellowHist = Histogram::ColourHistogram(BrickCV::BrickColour::RED, BrickCV::ChannelType::LAB_BLUE_YELLOW, labROIch[2]);

    auto redAxisHist = axisHist.getHistogram().lock(); 
    if (redAxisHist != nullptr)
    {
        cv::Mat redHistObj = redAxisHist->getHistogram();

        float hrange[] = { 0.0, 256.0 };
        const float* range[] = { hrange };
        int channels[] = { 0 };
        cv::calcBackProject(axisCh, 1, channels, redHistObj, colourMatch, range);
        cv::threshold(colourMatch, colourMatch, threshold, 255, cv::THRESH_BINARY);

        auto lockedBlueYellowHist = blueYellowHist.getHistogram().lock(); 
        if (lockedBlueYellowHist != nullptr)
        {
            cv::Mat colourMatch2; 
            cv::Mat blueYellowHistObj = lockedBlueYellowHist->getHistogram();

            cv::calcBackProject(byCh, 1, channels, blueYellowHistObj, colourMatch2, range);
            cv::threshold(colourMatch2, colourMatch2, threshold, 255, cv::THRESH_BINARY);

            cv::Mat result; 
            cv::add(colourMatch, colourMatch2, result, cv::Mat(), -1);

            cv::medianBlur(result, result, 9);
            return result; 
        }
        else
        {
            Errors::ErrorOutput(Errors::BrickCVErrors::NULL_PTR, "Colourhistogram returned nullptr upon lock operation for back projection");
        }
        cv::medianBlur(colourMatch, colourMatch, 9);
        return colourMatch;
    }
    else
    {
        Errors::ErrorOutput(Errors::BrickCVErrors::NULL_PTR, "Colourhistogram returned nullptr upon lock operation for back projection");
    }

    return cv::Mat(); 
}

cv::Mat ImageProcessing::ImageProcessor::backprojectHistogramHSV(cv::Mat& inputImage, cv::Mat& regionOfInterest, int threshold)
{
    //convert input image and ROI to lab and split channels
    //we seperate out the A & B channel
    cv::Mat hsvImage;
    cv::Mat labROI;
    cv::cvtColor(inputImage, hsvImage, cv::COLOR_BGR2HSV);
    cv::Mat colourMatch; //outputmask
    colourMatch.create(hsvImage.size(), hsvImage.depth());

    cv::Mat hsvImageCh[3];
    cv::split(hsvImage, hsvImageCh);
    cv::Mat labROIch[3];
    cv::split(labROI, labROIch);
    cv::Mat* hueCh = &hsvImageCh[1];

    cv::waitKey(0);

    auto hueHist = Histogram::ColourHistogram(BrickCV::BrickColour::RED, BrickCV::ChannelType::LAB_AXIS, labROIch[1]);

    auto redAxisHist = hueHist.getHistogram().lock();
    if (redAxisHist != nullptr)
    {
        cv::Mat redHistObj = redAxisHist->getHistogram();

        float hrange[] = { 0.0, 256.0 };
        const float* range[] = { hrange };
        int channels[] = { 0 };
        cv::calcBackProject(hueCh, 1, channels, redHistObj, colourMatch, range);
        cv::threshold(colourMatch, colourMatch, threshold, 255, cv::THRESH_BINARY);
        return colourMatch;
    }
    else
    {
        Errors::ErrorOutput(Errors::BrickCVErrors::NULL_PTR, "Colourhistogram returned nullptr upon lock operation for back projection");
    }

    return cv::Mat();
}

cv::Mat ImageProcessing::ImageProcessor::createThresholdMask(cv::Mat& greyImage)
{
    cv::Mat greyScale; 
    cv::GaussianBlur(greyImage, greyScale, cv::Size(), 1.4);
    cv::threshold(greyScale, greyScale, 150, 255, cv::ADAPTIVE_THRESH_MEAN_C);
    return greyScale;
}

//resizes large images to the vertical size of 50 pxls
cv::Mat ImageProcessing::ImageProcessor::reseizeImage(cv::Mat& image)
{
    auto numVerticalPixl = image.rows;
    auto desiredSizeFactorVertical = numVerticalPixl / 500;
    if (desiredSizeFactorVertical > 1)
    {
        auto desiredSize = numVerticalPixl / desiredSizeFactorVertical;
        float reseizeFactor = static_cast<float>(desiredSize) / static_cast<float>(numVerticalPixl);
        cv::Mat resized;
        cv::resize(image, resized, cv::Size(), reseizeFactor, reseizeFactor, cv::INTER_LINEAR);
        return resized;
    }
    return image;
}

cv::Mat ImageProcessing::ImageProcessor::thresholdColourOnChannel(cv::Mat channel, int lowerBound, int upperBound, const char* frameName, bool showImage /*= false */)
{
    cv::Mat output; 
    cv::threshold(channel,output, lowerBound, upperBound, cv::NORM_MINMAX);
    cv::imshow(frameName, output);
    cv::waitKey(0);
    return output; 
}

// Dev function
void ImageProcessing::ImageProcessor::debugInfo(cv::Mat& image)
{
    std::cout << "image channels: "   << image.channels() << std::endl;
    std::cout << "image cols: "       << image.cols << std::endl;
    std::cout << "image rows: "       << image.rows << std::endl;
    std::cout << "image dimensions: " << image.dims << std::endl;
}

// Function based on OpenCV 4 Computer Vision Application cookbook
// Sharpen edges
cv::Mat ImageProcessing::ImageProcessor::sharpen2Dedges(cv::Mat& image)
{
    //The kernal determines by how much the surrounding neighbour values will be adjusted to achieve sharpened appearance
    cv::Mat kernel(3, 3, CV_32F, cv::Scalar(0));

    kernel.at<float>(1, 1) =  5.0;
    kernel.at<float>(0, 1) = -1.0;
    kernel.at<float>(2, 1) = -1.0;
    kernel.at<float>(1, 0) = -1.0;
    kernel.at<float>(1, 2) = -1.0;

    cv::Mat result;
    cv::filter2D(image, result, image.depth(), kernel);

    return result;
}

// Function based on OpenCV 4 Computer Vision Application cookbook
// returns copy of image passed in with pixel values devided to fit desired colour space 
cv::Mat ImageProcessing::ImageProcessor::rgbColourSpaceReductionWithIt(cv::Mat& image, int divideBy)
{
    auto it = image.begin<cv::Vec3b>();
    for (; it != image.end<cv::Vec3b>(); it++)
    {
        (*it)[0] = (*it)[0] % divideBy + (divideBy / 2);
        (*it)[1] = (*it)[1] % divideBy + (divideBy / 2);
        (*it)[2] = (*it)[2] % divideBy + (divideBy / 2);
    }
    return image;
}

// Function based on OpenCV 4 Computer Vision Application cookbook
// returns copy of image passed in with pixel values devided to fit desired colour space 
cv::Mat ImageProcessing::ImageProcessor::naiveRgbColourSpaceReduction(cv::Mat& image, int divideBy)
{
    auto rows = image.rows;
    auto cols = image.channels() * image.cols;

    if (image.isContinuous())
    {
        cols = cols * rows;
        rows = 1;
    }

    //loop over all pixels in the image and reduce colour value on each cannel
    for (auto rowIndex = 0; rowIndex < rows; rowIndex++)
    {
        auto colourChannelAdress = image.ptr<uchar>(rowIndex);
        for (auto columnIndex = 0; columnIndex < cols; columnIndex++)
        {
            // For each channel we apply a reduction based on how many different colours we want to identify
            // If we want to identify 32 colours, we divide by 8, for 16 -> 16 ect...
            // We add a small factor to get definitives for any values on the edge between two colours
            colourChannelAdress[columnIndex] = (colourChannelAdress[columnIndex] / divideBy * divideBy + (divideBy / 2));
        }
    }
    return image;
}

// Function based on OpenCV 4 Computer Vision Application cookbook
// returns copy of image passed in with pixel values devided to fit desired colour space 
cv::Mat ImageProcessing::ImageProcessor::naiveRgbColourSpaceReduction2(cv::Mat& image, int divideBy)
{
    auto rows = image.rows;
    auto cols = image.channels() * image.cols;

    if (image.isContinuous())
    {
        cols = cols * rows;
        rows = 1;
    }

    //loop over all pixels in the image and reduce colour value on each cannel
    for (auto rowIndex = 0; rowIndex < rows; rowIndex++)
    {
        auto colourChannelAdress = image.ptr<uchar>(rowIndex);
        for (auto columnIndex = 0; columnIndex < cols; columnIndex++)
        {
            // For each channel we apply a reduction based on how many different colours we want to identify
            // If we want to identify 32 colours, we divide by 8, for 16 -> 16 ect...
            // We add a small factor to get definitives for any values on the edge between two colours
            colourChannelAdress[columnIndex] = colourChannelAdress[columnIndex] - colourChannelAdress[columnIndex] % divideBy + (divideBy / 2);
        }
    }
    return image;
}

// Function based on OpenCV 4 Computer Vision Application cookbook
// returns copy of image passed in with pixel values devided to fit desired colour space 
cv::Mat ImageProcessing::ImageProcessor::bitwiseRgbColourSpaceReduction(cv::Mat& image, int divideBy)
{
    auto rows = image.rows;
    auto cols = image.channels() * image.cols;

    if (image.isContinuous())
    {
        cols = cols * rows;
        rows = 1;
    }

    //loop over all pixels in the image and reduce colour value on each cannel
    for (auto rowIndex = 0; rowIndex < rows; rowIndex++)
    {
        auto colourChannelAdress = image.ptr<uchar>(rowIndex);
        for (auto columnIndex = 0; columnIndex < cols; columnIndex++)
        {
            // For each channel we apply a reduction based on how many different colours we want to identify
            // If we want to identify 32 colours, we divide by 8, for 16 -> 16 ect...
            // We add a small factor to get definitives for any values on the edge between two colours

            uchar mask = 0xFF << divideBy;
            *colourChannelAdress &= mask;
            *colourChannelAdress++ += (divideBy >> 1);
        }
    }
    return image;
}

// Function returns the absolute value between the three colour channels in a Vec3b object
// Function based on OpenCV 4 Computer Vision Application cookbook
int ImageProcessing::ImageProcessor::getDistanceToTargetColour(const cv::Vec3b& colourIn, const cv::Vec3b& tragetColour) const
{
    return abs(colourIn[0] - tragetColour[0])
         + abs(colourIn[1] - tragetColour[1])
         + abs(colourIn[2] - tragetColour[2]); 
}

