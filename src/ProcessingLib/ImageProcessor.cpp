#include "ImageProcessor.h"
#include "Histogram/SingleChannelHistogram.h"

//std includes
#include <iostream>
#include <memory>

//computer vision library incudes
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "ERRORs/ErrorOutput.h"

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
            boundRect[i] = boundingRect(contours_poly[i]);
        }
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
    about1Square = about1Square * 0.95; 

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

cv::Mat PreProcessor::ImageProcessor::applySobel(cv::Mat& blurredBGR, int k)
{
    cv::Mat blurredGreyscale; 
    cv::Mat sobelx, sobely, gradient;
    cv::cvtColor(blurredBGR, blurredGreyscale, cv::COLOR_BGR2GRAY);
    cv::Sobel(blurredGreyscale, sobelx, CV_64F, 1, 0, k);
    cv::Sobel(blurredGreyscale, sobely, CV_64F, 0, 1, k);
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

    cv::Mat imageAB; 
    cv::Mat brickSampleAB;
    imageAB.create(labImage.size(), labImage.depth());
    brickSampleAB.create(brickSampleAB.size(), brickSampleAB.depth());
    std::vector<float> ranges = { 0,255,0,255};


    //Seperated out A & B channels for both input and sample
    int channels[4] = {1,0,2,1};
    cv::mixChannels(&labImage, 1, &imageAB, 1, channels, 2);
    cv::mixChannels(&labImage, 1, &imageAB, 1, channels, 2);

    std::vector<int> histCh = {0,1}; 
    std::vector<int> histSize = { 256, 256};
    cv::Mat brickHist;
    cv::calcHist(brickSampleAB, histCh,cv::Mat(), brickHist, histSize, ranges);
    cv::normalize(brickHist,brickHist, 1.0);

    std::vector<int> numImage = {1};
    cv::InputArrayOfArrays images = {imageAB};
    cv::calcBackProject(images, histCh,brickHist,colourMatch,ranges,1);
    cv::threshold(colourMatch, colourMatch, threshold, 255, cv::THRESH_BINARY);

    return colourMatch;
}

// void PreProcessor::ImageProcessor::displayAllColourModels()
// {
//     if (this->visualiserInstance != nullptr)
//     {
//         visualiserInstance->displayRGBChannels();
//         visualiserInstance->displayHSVChannels();
//         visualiserInstance->displayLABChannels();
//         visualiserInstance->displayLUVChannels();
//     }
//     else
//     {
//         Errors::ErrorOutput(Errors::BrickCVErrors::NULL_PTR, "This ImageProcessor instance' visualiseInstance is a nullptr. Was it called too early?"); 
//     }
// }

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

