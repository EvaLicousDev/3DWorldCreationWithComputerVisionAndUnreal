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

// Displays copy of image reference resized 
void PreProcessor::ImageProcessor::display(cv::Mat& image, const char* displayName,int pixels)
{
    if (!this->in_image.empty())
    {
        this->debugInfo(image);
        cv::namedWindow(displayName);
        // Create an empty Mat object for the resized image
        cv::Mat resizedImage;
        // Resize the image
        cv::resize(image, resizedImage, cv::Size(pixels, pixels));
        cv::imshow(displayName, resizedImage);
        cv::waitKey(0);
    }
    else
    {
        std::cout << "Could not display image - please breakpoint and debug" << std::endl;
        std::cin.get();
    }
}

// Displays image reference. By defualt it sizes the picture down to 65% of it's original size. This can be toggled off by pasig in "false"
void PreProcessor::ImageProcessor::display(cv::Mat& image, const char* displayName, bool sizeDown /* = true */)
{
    if (!this->in_image.empty())
    {
        this->debugInfo(image);
        cv::namedWindow(displayName);
        if (sizeDown)
        {
            cv::Mat resizedImage;
            cv::resize(image, resizedImage, cv::Size(), 0.65, 0.65, cv::INTER_LINEAR);
            cv::imshow(displayName, resizedImage);
            cv::waitKey(0);
            return; 
        }
        cv::imshow(displayName, image);
        cv::waitKey(0);
    }
    else
    {
        std::cout << "Could not display image - please breakpoint and debug" << std::endl;
        std::cin.get();
    }
}

//Function returns the absolute value between the three colour channels in a Vec3b object
int PreProcessor::ImageProcessor::getDistanceToTargetColour(const cv::Vec3b& colourIn, const cv::Vec3b& tragetColour) const
{
    return abs(colourIn[0] - tragetColour[0])
         + abs(colourIn[1] - tragetColour[1])
         + abs(colourIn[2] - tragetColour[2]); 
}

void PreProcessor::ImageProcessor::displayHSVChannels()
{
    if (!this->in_image.empty())
    {
        cv::namedWindow("Hue");
        cv::namedWindow("Saturation");
        cv::namedWindow("Value");

        cv::Mat resizedHue;
        cv::resize(image_hue, resizedHue, cv::Size(), 0.35, 0.35, cv::INTER_LINEAR);
        cv::imshow("Hue", resizedHue);

        cv::Mat resizedSaturation;
        cv::resize(image_saturation, resizedSaturation, cv::Size(), 0.35, 0.35, cv::INTER_LINEAR);
        cv::imshow("Saturation", resizedSaturation);

        cv::Mat resizedValue;
        cv::resize(image_value, resizedValue, cv::Size(), 0.35, 0.35, cv::INTER_LINEAR);
        cv::imshow("Value", resizedValue);

        cv::waitKey(0);
        return;

    }
    else
    {
        std::cout << "Could not display image - please breakpoint and debug" << std::endl;
        std::cin.get();
    }
}


void PreProcessor::ImageProcessor::displayLABChannels()
{
    if (!this->in_image.empty())
    {
        cv::namedWindow("Lumosity");
        cv::namedWindow("Axis");
        cv::namedWindow("B_Y");

        cv::Mat resizedL;
        cv::resize(image_Lumosity, resizedL, cv::Size(), 0.35, 0.35, cv::INTER_LINEAR);
        cv::imshow("Lumosity", resizedL);

        cv::Mat resizedA;
        cv::resize(image_Axis, resizedA, cv::Size(), 0.35, 0.35, cv::INTER_LINEAR);
        cv::imshow("Axis", resizedA);

        cv::Mat resizedB;
        cv::resize(image_BlueYellow, resizedB, cv::Size(), 0.35, 0.35, cv::INTER_LINEAR);
        cv::imshow("B_Y", resizedB);

        cv::waitKey(0);
        return;

    }
    else
    {
        std::cout << "Could not display image - please breakpoint and debug" << std::endl;
        std::cin.get();
    }
}


void PreProcessor::ImageProcessor::displayLUVChannels()
{
    if (!this->in_image.empty())
    {
        cv::namedWindow("Lumosity2");
        cv::namedWindow("U_Channel");
        cv::namedWindow("V_Channel");

        cv::Mat resizedL;
        cv::resize(image_Lumosity2, resizedL, cv::Size(), 0.35, 0.35, cv::INTER_LINEAR);
        cv::imshow("Lumosity2", resizedL);

        cv::Mat resizedU;
        cv::resize(image_U, resizedU, cv::Size(), 0.35, 0.35, cv::INTER_LINEAR);
        cv::imshow("U_Channel", resizedU);

        cv::Mat resizedV;
        cv::resize(image_V, resizedV, cv::Size(), 0.35, 0.35, cv::INTER_LINEAR);
        cv::imshow("V_Channel", resizedV);

        cv::waitKey(0);
        return;
    }
    else
    {
        std::cout << "Could not display image - please breakpoint and debug" << std::endl;
        std::cin.get();
    }
}

void PreProcessor::ImageProcessor::displayRGBChannels()
{
    if (!this->in_image.empty())
    {
        cv::namedWindow("Red");
        cv::namedWindow("Green");
        cv::namedWindow("Blue");

        cv::Mat resizedR;
        cv::resize(image_R, resizedR, cv::Size(), 0.35, 0.35, cv::INTER_LINEAR);
        cv::imshow("Red", resizedR);

        cv::Mat resizedG;
        cv::resize(image_G, resizedG, cv::Size(), 0.35, 0.35, cv::INTER_LINEAR);
        cv::imshow("Green", resizedG);

        cv::Mat resizedB2;
        cv::resize(image_B, resizedB2, cv::Size(), 0.35, 0.35, cv::INTER_LINEAR);
        cv::imshow("Blue", resizedB2);

        cv::waitKey(0);
        return;
    }
    else
    {
        std::cout << "Could not display image - please breakpoint and debug" << std::endl;
        std::cin.get();
    }
}

void PreProcessor::ImageProcessor::updateHSVChannelsWithProcessed(cv::Mat& processed)
{
    std::vector<cv::Mat> rgb_channels;
    cv::split(processed, rgb_channels);
    image_R = rgb_channels[2];
    image_G = rgb_channels[1];
    image_B = rgb_channels[0];

    cv::Mat hsv;
    cv::cvtColor(processed, hsv, cv::COLOR_BGR2HSV);
    std::vector<cv::Mat> hsv_channels;
    cv::split(hsv, hsv_channels);
    image_hue = hsv_channels[0];
    image_saturation = hsv_channels[1];
    image_value = hsv_channels[2];

    //seperate and retrieve lap colour channels
    cv::Mat lab;
    cv::cvtColor(processed, lab, cv::COLOR_BGR2Lab);
    std::vector<cv::Mat> lab_channels;
    cv::split(lab, lab_channels);
    image_Lumosity = lab_channels[0];
    image_Axis = lab_channels[1];
    image_BlueYellow = lab_channels[2];

    //seperate and retrieve luv colour channels
    cv::Mat luv;
    cv::cvtColor(processed, luv, cv::COLOR_BGR2Luv);
    std::vector<cv::Mat> luv_channels;
    cv::split(luv, luv_channels);
    image_Lumosity2 = luv_channels[0];
    image_U = luv_channels[1];
    image_V = luv_channels[2];
}

//using the image which the class was initialised with, we determine the distance to pixels in the image to a target colour
//TO DO: dynamically allocate max distance
//TO DO: TEST
//cv::Mat PreProcessor::ImageProcessor::createColourLocationImageBW(int maxDistance, const cv::Vec3b& targetColour)
//{
//    //create an 8 bit image with 1 colour channel
//    auto output = cv::Mat(in_image.rows, in_image.cols, CV_8UC1);
//
//    //get colour channels
//     cv::Mat_<cv::Vec3b>::const_iterator imageIt  = in_image.begin; 
//     cv::Mat_<cv::Vec3b>::const_iterator intended = in_image.end; 
//     auto resultIt = output.begin<uchar>();
//     
//     //for each pixel compute absolute distance, and if value is within range of colour set output pixel to white, else black
//     for (; imageIt != intended; imageIt++, resultIt++)
//     {
//         (getDistanceToTargetColour(*imageIt, targetColour) <= maxDistance) ? *resultIt = 255 : *resultIt = 0; 
//     }
//
//     return output; 
//}

