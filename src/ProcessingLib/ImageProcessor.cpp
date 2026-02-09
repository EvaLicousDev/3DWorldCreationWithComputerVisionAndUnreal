#include "ImageProcessor.h"

//std includes
#include <iostream>

//computer vision library incudes
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

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

// Displays copy of resized main image if no other data is provided
void PreProcessor::ImageProcessor::display(int widthPixels, int heightPixels)
{
    if (!this->in_image.empty())
    {
        cv::namedWindow("Target");
        // Create an empty Mat object for the resized image
        cv::Mat resizedImage;
        // Resize the image
        cv::resize(in_image, resizedImage, cv::Size(widthPixels, heightPixels));
        cv::imshow("Target", resizedImage);
        cv::waitKey(0);
    }
    else
    {
        std::cout << "Could not display image - please breakpoint and debug" << std::endl;
        std::cin.get();
    }
}

// Displays copy of image reference resized 
void PreProcessor::ImageProcessor::display(cv::Mat& image, int pixels /* = 500 */)
{
    if (!this->in_image.empty())
    {
        this->debugInfo(image);
        cv::namedWindow("Processed");
        // Create an empty Mat object for the resized image
        cv::Mat resizedImage;
        // Resize the image
        cv::resize(image, resizedImage, cv::Size(pixels, pixels));
        cv::imshow("Processed", resizedImage);
        cv::waitKey(0);
    }
    else
    {
        std::cout << "Could not display image - please breakpoint and debug" << std::endl;
        std::cin.get();
    }
}
