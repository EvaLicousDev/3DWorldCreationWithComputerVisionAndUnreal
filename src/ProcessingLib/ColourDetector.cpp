#include "ColourDetector.h"


cv::Mat AbsColourDistance::ColourDetector::processChannel(const cv::Mat image, const cv::Mat& example, int& boundry, BrickCV::ChannelType channelType)
{
    boundry = MIN(boundry, 20); //Do not allow more than + - 20 in value
    cv::Mat output;
    cv::Mat* channel; 
    cv::Mat* roiChannel;

    cv::cvtColor(image, image, cv::COLOR_BGR2Lab); 
    cv::cvtColor(example,  example, cv::COLOR_BGR2Lab);

    cv::Mat labChannels[3];
    cv::split(image, labChannels);

    cv::Mat labROI[3];
    cv::split(example, labROI);

    if (channelType == BrickCV::ChannelType::LAB_AXIS)
    {
        channel = &labChannels[1];
        roiChannel = &labROI[1];
    }

    cv::absdiff(*channel, *roiChannel, output);
    std::vector<cv::Mat> images;
    cv::split(output, images);
    output = images[0] + images[1] + images[2];
    cv::threshold(output, 
        output, 
        boundry, 
        255,
        cv::THRESH_BINARY); 
    return output;
}

cv::Mat AbsColourDistance::ColourDetector::findPixelsWithColourInRange(const cv::Mat input, const cv::Scalar colour)
{
    cv::Mat labColour;
    cv::cvtColor(input, labColour, cv::COLOR_BGR2Lab);
    cv::Mat rgbToLab = cv::Mat(input.rows*0.2, input.cols*0.2, CV_8UC3, colour);
    cv::cvtColor(rgbToLab, rgbToLab, cv::COLOR_BGR2Lab);

    cv::Scalar colourScalar = cv::mean(rgbToLab); 
    cv::Scalar minThreshold = colourScalar * 0.90; 
    cv::Scalar maxThreshold = colourScalar * 1.10; 

    cv::Mat out; 

    cv::inRange(labColour, minThreshold, maxThreshold, out);
    cv::medianBlur(out, out, 9);
    return out;
}

//use euclidian distance to example HTML shades to decide colour
const BrickColour AbsColourDistance::ColourDetector::getBrickApproximation(const cv::Mat& colourSample)
{
    cv::Mat labColour;
    cv::cvtColor(colourSample, labColour, cv::COLOR_BGR2Lab);
    auto meanColour = cv::mean(labColour);
    BrickColour bestMatch = BrickColour::BLACK; 
    double distance = 9999999;

    for (const auto colour : vectorNames)
    {
        auto colourVector = exampleBrickShades[colour];
        if (m_brickColourMap == nullptr)
        {
            // If we don't have a brick sample scalar, we use selected default HTML colour shades
            // This method is prone to errors, as it does not account for light conditions being challanging 
            for (const auto shade : colourVector)
            {
                auto shadeScalar = getBGRColour(shade);
                cv::Mat rgbToLab = cv::Mat(colourSample.rows, colourSample.cols, CV_8UC3, shadeScalar);
                cv::cvtColor(rgbToLab, rgbToLab, cv::COLOR_BGR2Lab);
                shadeScalar = cv::mean(rgbToLab);

                double distanceToEachOther = cv::norm(shadeScalar, meanColour);
                if (distanceToEachOther < distance)
                {
                    distance = distanceToEachOther;
                    bestMatch = colour;
                }
            }
        }
        else
        {
            // If we have a brick sample scalar we use that
            auto found = m_brickColourMap->find(colour);
            if (found != m_brickColourMap->end())
            {
                auto brickShade = m_brickColourMap->at(colour);
                cv::Mat rgbToLab = cv::Mat(colourSample.rows, colourSample.cols, CV_8UC3, brickShade);
                cv::cvtColor(rgbToLab, rgbToLab, cv::COLOR_BGR2Lab);
                auto shadeScalar = cv::mean(rgbToLab);

                double distanceToEachOther = cv::norm(shadeScalar, meanColour);
                if (distanceToEachOther < distance)
                {
                    distance = distanceToEachOther;
                    bestMatch = colour;
                }
            }
        }
    }

    return bestMatch; 
}
