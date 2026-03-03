#include "ImageProcessor.h"
#include "Histogram/SingleChannelHistogram.h"

//std includes
#include <iostream>
#include <memory>

//computer vision library incudes
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d.hpp>

#include "ERRORs/ErrorOutput.h"
#include "Histogram/ColourHistogram.h"
#include "Histogram/HistorgramGraphCreator.h"

void PreProcessor::ImageProcessor::getContourData(cv::Mat& allEdgesAdded, bool drawContours)
{
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(allEdgesAdded, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
    std::vector<std::vector<cv::Point>> contours_poly(contours.size());
    std::vector<cv::Rect> boundRect(contours.size());
    std::vector<cv::Rect> filteredRect(contours.size());

    if (drawContours)
    {
        drawenContours.release();
        drawenContours = cv::Mat(allEdgesAdded.cols, allEdgesAdded.rows, CV_8UC1);
        for (size_t i = 0; i < contours.size(); i++)
        {
            approxPolyDP(contours[i], contours_poly[i], 4, true);
            // if (cv::isContourConvex(contours_poly[i]))
            // {
            //     boundRect[i] = boundingRect(contours_poly[i]);
            // }
            boundRect[i] = boundingRect(contours_poly[i]);
        }
        cv::drawContours(drawenContours, contours_poly, -1, CV_RGB(255,255,255)); 
    }

    int numBiggest = 0;
    int idBiggest  = 0;
    for (int i = 0; i < boundRect.size(); i++) {
        if (boundRect[i].width > numBiggest) {
            numBiggest = boundRect[i].width;
            idBiggest = i;
        }
    }

    double biggestWidth = boundRect[idBiggest].width;
    double about1Square = biggestWidth / 32;
    about1Square = about1Square * 0.98; 

    for (size_t i = 0; i < contours.size(); i++)
    {
        if (boundRect[i].width >= about1Square && boundRect[i].height >= about1Square)
        {
            filteredRect.emplace_back(boundRect[i]);
        }
    }

    this->m_boundRect = std::make_shared<std::vector<cv::Rect>>(filteredRect);
    this->m_contours_poly = std::make_shared<std::vector<std::vector<cv::Point>>>(contours_poly);
}

cv::Rect PreProcessor::ImageProcessor::findRectWithLongestSide(const std::vector<std::vector<cv::Point>>& contours, cv::Rect& topleftGreenCorner) {
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


cv::Rect PreProcessor::ImageProcessor::findRectWithLargestVolium(const std::vector<cv::Rect>& boxes) {
    cv::Rect greatestVolium;
    int maxVol = 0;

    for (const auto& box : boxes) {
        int currentMaxVol = box.area();

        if (currentMaxVol > maxVol) {
            maxVol =  currentMaxVol;
            greatestVolium = box;
        }
    }
    return greatestVolium;
}

cv::Mat PreProcessor::ImageProcessor::getMSERMask(cv::Mat unblurredImage)
{
    cv::Mat mserMask; 
    mserMask.create(unblurredImage.rows, unblurredImage.cols, CV_8UC1);
    cv::Ptr<cv::MSER> mserAlgorithm = cv::MSER::create();

    std::vector<std::vector<cv::Point> > regions;
    std::vector<cv::Rect> mser_bbox;
    mserAlgorithm->detectRegions(unblurredImage, regions, mser_bbox);

    for (std::vector<cv::Point> v : regions) {
        for (cv::Point p : v) {
            mserMask.at<uchar>(p.y, p.x) = 255;
        }
    }

    cv::threshold(mserMask, mserMask, 252, 255, cv::THRESH_BINARY); 
    this->mserMask = std::make_shared<cv::Mat>(mserMask); 
    return mserMask; 
}

cv::Mat PreProcessor::ImageProcessor::addGreenAndMSER(const cv::Mat& green, const cv::Mat& mser)
{
    if (green.rows != mser.rows || green.cols != mser.cols)
    {
        Errors::ErrorOutput(Errors::IMAGEs_NOT_EQUAL_SIZE, "Green mask and mser mask are expected dto be qual size but are not! Please investigate."); 
        return cv::Mat(); 
    }

    cv::Mat outMask = cv::Mat::zeros(green.rows, green.cols, CV_8UC1); 

    //custom add function
    auto rows = green.rows;
    auto cols = green.channels() * green.cols;

    auto bothContinuous = green.isContinuous() && mser.isContinuous(); 
    if (bothContinuous)
    {
        cols = cols * rows;
        rows = 1;
    }

    //loop over all pixels in the image and reduce colour value on each cannel
    for (auto rowIndex = 0; rowIndex < rows; rowIndex++)
    {
        auto greenPx = green.ptr<uchar>(rowIndex);
        auto mserPx = mser.ptr<uchar>(rowIndex);
        auto outPx = outMask.ptr<uchar>(rowIndex); 

        for (auto columnIndex = 0; columnIndex < cols; columnIndex++)
        {
            // For each channel we apply a reduction based on how many different colours we want to identify
            // If we want to identify 32 colours, we divide by 8, for 16 -> 16 ect...
            // We add a small factor to get definitives for any values on the edge between two colours
            if (greenPx[columnIndex]<255)
            {
                //favour black pixles of green mask, and make them black in output
                outPx[columnIndex] = 0; 
            } 
            else if (mserPx[columnIndex] < 255) //green is white but mser is black
            {
                //set a light grey value for non
                outPx[columnIndex] = 150;
            }
            else
            {
                outPx[columnIndex] = 255;
            }
        }
    }
    return outMask; 
}

cv::Rect PreProcessor::ImageProcessor::findLegoWithThresholdingMask(cv::Mat imageToProcess, int lowerboundGreen, bool showGreenMask /* = false */, bool showAllRect /* = false */) {

    cv::Mat legoPixMask;
    cv::Mat copy; 
    imageToProcess.copyTo(copy); 

    cv::threshold(copy, legoPixMask, 50, 255, cv::THRESH_BINARY);
    cv::cvtColor(legoPixMask, legoPixMask, cv::COLOR_BGR2GRAY);
    cv::threshold(legoPixMask, legoPixMask, 50, 255, cv::THRESH_BINARY);
    cv::medianBlur(legoPixMask, legoPixMask, 9); 

    this->legoPXMask = std::make_shared<cv::Mat>(legoPixMask); 
    
    if (showGreenMask)
    {
        cv::imshow("Green pix mask", legoPixMask);
        cv::waitKey(0);
    }

    //find contours in thresholded
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(legoPixMask, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
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

    //get largest by volium
    auto greatestInRed = this->findRectWithLargestVolium(boundRect); 

    copy.release(); 

    return greatestInRed;
}

cv::Rect PreProcessor::ImageProcessor::findBlackBG(cv::Mat imageToProcess, bool showBlackMask /* = false */, bool showAllRect /* = false */) {
    //isolate red channel and threshold
    cv::Mat greyScale;
    cv::cvtColor(imageToProcess, greyScale, cv::COLOR_BGR2GRAY);

    cv::Mat blackTrayMask;
    cv::inRange(greyScale, 0, 50, blackTrayMask);

    if (showBlackMask)
    {
        cv::imshow("Black Tray", blackTrayMask);
        cv::waitKey(0);
    }

    //find contours in thresholded
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(blackTrayMask, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
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

    //get largest by volium
    auto greatestInRed = this->findRectWithLargestVolium(boundRect);
    return greatestInRed;
}


// cv::Mat PreProcessor::ImageProcessor::customSobelEdges(cv::Mat& input1, cv::Mat& input2, cv::Mat& input3)
// {
//     return bestEdges(input1, input2, input3); 
// }

void PreProcessor::ImageProcessor::setContouringThresholds(cv::Mat& blurredGreyscale)
{
    normalize(blurredGreyscale, blurredGreyscale, 0, 255, cv::NORM_MINMAX);
    cv::minMaxIdx(blurredGreyscale, &controurThreshMin, &controurThreshMax, 0, 0);
    controurThreshMiddle = (controurThreshMin + controurThreshMax) / 2;
    double maxThreshCan = controurThreshMiddle * 1.5;
    if (maxThreshCan > 255)
        maxThreshCan = 255;
    double minThreshCan = maxThreshCan * 0.9;
}

cv::Mat PreProcessor::ImageProcessor::applyCannyToBGR(cv::Mat& blurredBGR)
{
    cv::Mat blurredGreyscale; 
    cv::cvtColor(blurredBGR, blurredGreyscale, cv::COLOR_BGR2GRAY); 
    setContouringThresholds(blurredGreyscale);
    cv::Canny(blurredGreyscale, blurredGreyscale, controurThreshMin, controurThreshMax);
    return blurredGreyscale; 
}

cv::Mat PreProcessor::ImageProcessor::applyCannyTo1D(cv::Mat& blurredGrey, int threshold)
{
    setContouringThresholds(blurredGrey);
    cv::Canny(blurredGrey, blurredGrey, threshold, controurThreshMax);
    return blurredGrey;
}

cv::Mat PreProcessor::ImageProcessor::applySobel(cv::Mat& mask, int k)
{
    cv::Mat sobelx, sobely, gradient;
    cv::Sobel(mask, sobelx, CV_64F, 1, 0, k);
    cv::Sobel(mask, sobely, CV_64F, 0, 1, k);
    cv::magnitude(sobelx, sobely, gradient);
    // Convert to 8-bit image
    cv::convertScaleAbs(gradient, gradient);
    return gradient;
}

cv::Mat PreProcessor::ImageProcessor::backprojectHistogram(cv::Mat& inputImage, cv::Mat& regionOfInterest, int threshold)
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

   // cv::imshow("image axis", *axisCh);
    cv::waitKey(0);

    auto axisHist = Histogram::ColourHistogram(BrickCV::BrickColour::RED, BrickCV::ChannelType::LAB_AXIS, labROIch[1]); 
    auto blueYellowHist = Histogram::ColourHistogram(BrickCV::BrickColour::RED, BrickCV::ChannelType::LAB_BLUE_YELLOW, labROIch[2]);

    auto redAxisHist = axisHist.getHistogram().lock(); 
    if (redAxisHist != nullptr)
    {
        cv::Mat redHistObj = redAxisHist->getHistogram();

        auto graphRed = Histogram::GraphCreator::createHistorgramGraph(redHistObj);
        //cv::imshow("ROI axis histogram", graphRed);
        //cv::waitKey(0);

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

            auto graph = Histogram::GraphCreator::createHistorgramGraph(blueYellowHistObj);
            cv::imshow("ROI axis histogram", graph);
            cv::waitKey(0);

            cv::calcBackProject(axisCh, 1, channels, redHistObj, colourMatch2, range);
            cv::threshold(colourMatch2, colourMatch2, threshold, 255, cv::THRESH_BINARY);

            cv::Mat result; 
            cv::add(colourMatch, colourMatch2, result, cv::Mat(), -1);

           // cv::imshow("Axis mask", colourMatch); 
            //cv::imshow("BY", colourMatch2); 
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

cv::Mat PreProcessor::ImageProcessor::createThresholdMask(cv::Mat& greyImage)
{
    cv::Mat greyScale; 
    cv::GaussianBlur(greyImage, greyScale, cv::Size(), 1.4);
    cv::threshold(greyScale, greyScale, 150, 255, cv::ADAPTIVE_THRESH_MEAN_C);
    return greyScale;
}

//resizes large images to the vertical size of 50 pxls
cv::Mat PreProcessor::ImageProcessor::reseizeImage(cv::Mat& image)
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

cv::Mat PreProcessor::ImageProcessor::thresholdColourOnChannel(cv::Mat channel, int lowerBound, int upperBound, const char* frameName, bool showImage /*= false */)
{
    cv::Mat output; 
    cv::threshold(channel,output, lowerBound, upperBound, cv::NORM_MINMAX);
    cv::imshow(frameName, output);
    cv::waitKey(0);
    return output; 
}

// Dev function
void PreProcessor::ImageProcessor::debugInfo(cv::Mat& image)
{
    std::cout << "image channels: "   << image.channels() << std::endl;
    std::cout << "image cols: "       << image.cols << std::endl;
    std::cout << "image rows: "       << image.rows << std::endl;
    std::cout << "image dimensions: " << image.dims << std::endl;
}

// Sharpen edges
cv::Mat PreProcessor::ImageProcessor::sharpen2Dedges(cv::Mat& image)
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

// returns copy of image passed in with pixel values devided to fit desired colour space 
cv::Mat PreProcessor::ImageProcessor::rgbColourSpaceReductionWithIt(cv::Mat& image, int divideBy)
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

// returns copy of image passed in with pixel values devided to fit desired colour space 
cv::Mat PreProcessor::ImageProcessor::naiveRgbColourSpaceReduction(cv::Mat& image, int divideBy)
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

// returns copy of image passed in with pixel values devided to fit desired colour space 
cv::Mat PreProcessor::ImageProcessor::naiveRgbColourSpaceReduction2(cv::Mat& image, int divideBy)
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

// returns copy of image passed in with pixel values devided to fit desired colour space 
cv::Mat PreProcessor::ImageProcessor::bitwiseRgbColourSpaceReduction(cv::Mat& image, int divideBy)
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

//Function returns the absolute value between the three colour channels in a Vec3b object
int PreProcessor::ImageProcessor::getDistanceToTargetColour(const cv::Vec3b& colourIn, const cv::Vec3b& tragetColour) const
{
    return abs(colourIn[0] - tragetColour[0])
         + abs(colourIn[1] - tragetColour[1])
         + abs(colourIn[2] - tragetColour[2]); 
}

