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

//use euclidian distance to example HTML shades to decide colour
const BrickColour AbsColourDistance::ColourDetector::getBrickApproximation(const cv::Mat& colourSample)
{
    cv::Scalar avarage = cv::mean(colourSample);
    auto meanColour = cv::mean(colourSample);
    BrickColour bestMatch = BrickColour::BLACK; 
    double distance = 9999999; 
    for (const auto colour : vectorNames)
    {
        auto colourVector = exampleBrickShades[colour];
        for (const auto shade : colourVector)
        {
            auto shadeScalar = getBGRColour(shade);
            double distanceToEachOther = cv::norm(shadeScalar, meanColour);
            if (distanceToEachOther < distance)
            {
                distance = distanceToEachOther; 
                bestMatch = colour; 
            }
        }
    }

    return bestMatch; 
}
