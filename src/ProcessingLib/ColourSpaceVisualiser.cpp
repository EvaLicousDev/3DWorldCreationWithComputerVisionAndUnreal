#include "ColourSpaceVisualiser.h"

#include <iostream>
#include <opencv2/highgui.hpp>

void PreProcessor::ColourSpaceVisualiser::updateHSVChannelsWithProcessed(cv::Mat& processed)
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

void PreProcessor::ColourSpaceVisualiser::displayHSVChannels()
{
    if (!this->imageOfLego.empty())
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


void PreProcessor::ColourSpaceVisualiser::displayLABChannels()
{
    if (!this->imageOfLego.empty())
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


void PreProcessor::ColourSpaceVisualiser::displayLUVChannels()
{
    if (!this->imageOfLego.empty())
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

void PreProcessor::ColourSpaceVisualiser::displayRGBChannels()
{
    if (!this->imageOfLego.empty())
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

// Displays copy of image reference resized 
void PreProcessor::ColourSpaceVisualiser::display(cv::Mat& image, const char* displayName, int pixels)
{
    if (!this->imageOfLego.empty())
    {
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
void PreProcessor::ColourSpaceVisualiser::display(const cv::Mat& image, const char* displayName, bool sizeDown /* = true */)
{
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