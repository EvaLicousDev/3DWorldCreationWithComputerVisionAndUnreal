#include "ColourDetector.h"

#include <cmath>
#include <opencv2/imgproc/imgproc.hpp>


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

cv::Mat AbsColourDistance::ColourDetector::findPixelsWithColourInRange(const cv::Mat input, const cv::Scalar colour, double minMargine, double maxMargine)
{
    cv::Mat labColour;
    cv::cvtColor(input, labColour, cv::COLOR_BGR2Lab);
    cv::Mat rgbToLab = cv::Mat(static_cast<int>(input.rows), static_cast<int>(input.cols), CV_8UC3, colour);
    cv::cvtColor(rgbToLab, rgbToLab, cv::COLOR_BGR2Lab);

    cv::Scalar colourScalar = cv::mean(rgbToLab); 
    cv::Scalar minThreshold = colourScalar * minMargine; 
    cv::Scalar maxThreshold = colourScalar * maxMargine; 

    cv::Mat out; 

    cv::inRange(labColour, minThreshold, maxThreshold, out);
    cv::medianBlur(out, out, 9);
    return out;
}

//Function works like findPixelsWithColoursInRange(), but for single channel images in the LAB or LUV colour space and. If LCHuv colour space is used first input has to be LAB A-channel, and optional input has to be B-channel 
cv::Mat AbsColourDistance::ColourDetector::findPixelsWithColourInRangeForChannel(const cv::Mat inputBGR, const cv::Scalar colourBGR, double minMargine, double maxMargine, ChannelType channel, bool showResult)
{
    cv::Mat out;
    cv::Mat img;
    cv::Mat scal;
    cv::Mat colourSpace;
    cv::Mat individualChannelImg[3];
    cv::Mat individualChannelScalar[3];
    cv::Mat scalarImg = cv::Mat(inputBGR.rows, inputBGR.cols, CV_8UC3, colourBGR);

    if (channel == ChannelType::LAB_LUMINANCE || channel == ChannelType::LAB_AXIS || channel == ChannelType::LAB_BLUE_YELLOW)
    {
        cv::cvtColor(inputBGR, colourSpace, cv::COLOR_BGR2Lab);
        cv::cvtColor(scalarImg, scalarImg, cv::COLOR_BGR2Lab);
        cv::split(colourSpace, individualChannelImg); 
        cv::split(scalarImg, individualChannelScalar);
    }
    else if (channel == ChannelType::LUV_LUMINANCE || channel == ChannelType::LUV_UNIFORM_REDGREEN || channel == ChannelType::LUV_V_BLUEYELLOW)
    {
        cv::cvtColor(inputBGR, colourSpace, cv::COLOR_BGR2Luv);
        cv::cvtColor(scalarImg, scalarImg, cv::COLOR_BGR2Lab);
        cv::split(colourSpace, individualChannelImg);
        cv::split(scalarImg, individualChannelScalar);

    }
    else if (channel == ChannelType::LCHuv_LUMINANCE || channel == ChannelType::LCHuv_CHROMA || channel == ChannelType::LCHuv_HUE)
    {
        ErrorOutput(BrickCVErrors::COMPUT_MORE_EFFICENT_OPT_AVAILABLE, "The function call findPixelsWithColourInRangeForChannel() for LCHuv is valid, but expensive. Consider LUV or LAB.");

        if (channel == LCHuv_HUE)
        {
            //Design choice: we do not want Hues with a greater degree than 25, therefor we limit the degree selection for the Hue channls
            // 360 * (7/100) = 25.2
            minMargine = std::max<double>(minMargine, 0.96);
            maxMargine = std::min<double>(maxMargine, 1.03);
        }

        // Open CV does not accomodate LCHuv, so we implement custom logic here and work with LAB 
        // Not to calculate a singel channel here we need both A & B Lab coolour channels
        // https://www.chnspec.net/What-is-the-Lab-Color-Space-LCh.html#:~:text=Lab:%20Uses%20Cartesian%20coordinates%20to,%2C%20and%20Hue%20(h)
        // A call to this is however very expensive, both memory and operations wise

        cv::cvtColor(inputBGR, colourSpace, cv::COLOR_BGR2Lab);
        cv::cvtColor(scalarImg, scalarImg, cv::COLOR_BGR2Lab);

        cv::split(colourSpace, individualChannelImg);
        cv::split(scalarImg, individualChannelScalar); 
    }
    else
    {
        ErrorOutput(Errors::BrickCVErrors::WRONG_COLOUR_CHANNEL, "findPixelsWithColourInRangeForChannel() expects to use a single channel from a LAB or LUV or LCHuv colour image.");
        return out;
    }

    // Get the channel we want 
    if (channel == ChannelType::LCHuv_LUMINANCE || channel == ChannelType::LAB_LUMINANCE || channel == ChannelType::LUV_LUMINANCE)
    {
        // same for all 3 models
        img = individualChannelImg[0];
        scal = individualChannelScalar[0];
    }
    else if (channel == ChannelType::LUV_UNIFORM_REDGREEN || channel == ChannelType::LCHuv_CHROMA || channel == ChannelType::LAB_AXIS)
    {
        if (channel == ChannelType::LCHuv_CHROMA)
        {
            img = getLCHuvCHROMAMat(individualChannelImg[1], individualChannelImg[2], channel);
            scal = getLCHuvCHROMAMat(individualChannelScalar[1], individualChannelScalar[2], channel);
        }
        else
        {
            img = individualChannelImg[1];
            scal = individualChannelScalar[1];
        }
    }
    else if (channel == ChannelType::LCHuv_HUE || channel == ChannelType::LUV_V_BLUEYELLOW || channel == ChannelType::LAB_BLUE_YELLOW)
    {
        if (channel == ChannelType::LCHuv_HUE)
        {
            img = getLCHuvHUEMat(individualChannelImg[1], individualChannelImg[2], channel);
            scal = getLCHuvHUEMat(individualChannelScalar[1], individualChannelScalar[2], channel);
        }
        else
        {
            img = individualChannelImg[2];
            scal = individualChannelScalar[2];
        }
    }

    cv::threshold(img, img, minMargine, maxMargine, cv::THRESH_BINARY); 
    cv::medianBlur(out, out, 9);

    if (showResult)
    {
        std::string name{ getChannelName(channel) + std::to_string(minMargine) + " - " + std::to_string(maxMargine)};
        cv::imshow(name.c_str(), img);
        cv::waitKey(0);
    }

    return out;
}

// This function uses individual channels to threshold pixels in range per channel for a selected colour model LAB, LUV or LCHuv. Not LCHuv uses custom logic and is expensive
cv::Mat AbsColourDistance::ColourDetector::findPixelsWithColourInRangeIndividualChannels(
    const cv::Mat& inputBGR, 
    const cv::Scalar& colourBGR, 
    const ColourSpace colourSpace,
    double minMargineC1, // = 0.9, 
    double maxMargineC1, // = 1.1, 
    double minMargineC2, // = 0.9, 
    double maxMargineC2, // = 1.1, 
    double minMargineC3, // = 0.9, 
    double maxMargineC3, // = 1.1, 
    bool showResult //= false
)
{
    if (inputBGR.dims != 3)
    {
        ErrorOutput(Errors::BrickCVErrors::IMAGE_NOT_3_CHANNEL, "Image has to be 3 channel BGR image. \n Breakpoint and inspect input for findPixelsWithColourInRangeIndividualChannels()");
    }

    if (colourSpace == ColourSpace::LAB || colourSpace == ColourSpace::LUV || colourSpace == ColourSpace::LCHuv)
    {
        cv::Mat outMask;
        cv::Mat firstChannel; 
        cv::Mat secondChannel;
        cv::Mat thirdChannel;

        //get chosen images for colour space by channel
        if (colourSpace == ColourSpace::LAB)
        {
            firstChannel  = findPixelsWithColourInRangeForChannel(inputBGR, colourBGR, minMargineC1, maxMargineC1, ChannelType::LAB_LUMINANCE, showResult); 
            secondChannel = findPixelsWithColourInRangeForChannel(inputBGR, colourBGR, minMargineC2, maxMargineC2, ChannelType::LAB_AXIS, showResult);
            thirdChannel  = findPixelsWithColourInRangeForChannel(inputBGR, colourBGR, minMargineC3, maxMargineC3, ChannelType::LAB_BLUE_YELLOW, showResult);
        } 
        else if (colourSpace == ColourSpace::LUV)
        {
            firstChannel  = findPixelsWithColourInRangeForChannel(inputBGR, colourBGR, minMargineC1, maxMargineC1, ChannelType::LUV_LUMINANCE, showResult);
            secondChannel = findPixelsWithColourInRangeForChannel(inputBGR, colourBGR, minMargineC2, maxMargineC2, ChannelType::LUV_UNIFORM_REDGREEN, showResult);
            thirdChannel  = findPixelsWithColourInRangeForChannel(inputBGR, colourBGR, minMargineC3, maxMargineC3, ChannelType::LUV_V_BLUEYELLOW, showResult);
        }
        else if (colourSpace == ColourSpace::LCHuv)
        {
            firstChannel  = findPixelsWithColourInRangeForChannel(inputBGR, colourBGR, minMargineC1, maxMargineC1, ChannelType::LCHuv_LUMINANCE, showResult);
            secondChannel = findPixelsWithColourInRangeForChannel(inputBGR, colourBGR, minMargineC2, maxMargineC2, ChannelType::LCHuv_CHROMA, showResult);
            thirdChannel  = findPixelsWithColourInRangeForChannel(inputBGR, colourBGR, minMargineC3, maxMargineC3, ChannelType::LCHuv_HUE, showResult);
        }

        //normalize ranges so we can add them into 1 image
        cv::normalize(firstChannel, firstChannel, 0.0, 255.0, cv::NORM_MINMAX);
        cv::normalize(secondChannel, secondChannel, 0.0, 255.0, cv::NORM_MINMAX);
        cv::normalize(thirdChannel, thirdChannel, 0.0, 255.0, cv::NORM_MINMAX);

        cv::add(firstChannel, secondChannel, outMask);
        cv::threshold(outMask, outMask, 5, 255, cv::THRESH_BINARY); 
        cv::add(outMask, thirdChannel, outMask);
        cv::threshold(outMask, outMask, 5, 255, cv::THRESH_BINARY);

        if (showResult)
        {
            std::string frame{ "C1: " + std::to_string(minMargineC1) + " - " + std::to_string(maxMargineC1) 
                            + " C2: " + std::to_string(minMargineC2) + " - " + std::to_string(maxMargineC2) 
                            + " C3: " + std::to_string(minMargineC3) + " - " + std::to_string(maxMargineC3) };
            cv::imshow(frame.c_str(), outMask);
            cv::waitKey(0);
        }

        return outMask; 
    }
    else
    {
        ErrorOutput(Errors::BrickCVErrors::COLOUR_SPACE_MISMATCH, "This function - findPixelsWithColourInRangeIndividualChannels() - expects LAB, LUV or LCHuv");
    }
}

cv::Mat AbsColourDistance::ColourDetector::getLCHuvCHROMAMat(const cv::Mat& labAChannel, const cv::Mat& labBChannel, const ChannelType channelType)
{
    if (channelType != ChannelType::LCHuv_CHROMA)
    {
        ErrorOutput(Errors::BrickCVErrors::WRONG_COLOUR_CHANNEL, "getLCHuvCHROMAMat() called for channel of type", getChannelName(channelType));
        return cv::Mat(); 
    }

    if (labAChannel.rows != labBChannel.rows || labBChannel.cols != labAChannel.cols)
    {
        Errors::ErrorOutput(Errors::IMAGEs_NOT_EQUAL_SIZE, "Input for this function has to have equal rows and columns but didn't.");
        return cv::Mat();
    }

    cv::Mat outMask = cv::Mat::zeros(labAChannel.rows, labBChannel.cols, CV_64F);

    auto rows = labAChannel.rows;
    auto cols = labBChannel.channels() * labBChannel.cols;

    auto bothContinuous = labAChannel.isContinuous() && labBChannel.isContinuous();
    if (bothContinuous)
    {
        cols = cols * rows;
        rows = 1;
    }
    //convert to suitable datatype
    cv::Mat labA;
    cv::Mat labB;
    labAChannel.convertTo(labA, CV_32F); 
    labBChannel.convertTo(labB, CV_32F);

    //loop over all pixels in the images and apply transform to outpu
    for (auto rowIndex = 0; rowIndex < rows; rowIndex++)
    {
        auto labAPx = labA.ptr<float>(rowIndex);
        auto labBPx = labB.ptr<float>(rowIndex);
        auto outPx = outMask.ptr<float>(rowIndex);

        for (auto columnIndex = 0; columnIndex < cols; columnIndex++)
        {
            // Chroma -> C = sqrt((a^2 + b^2)) - https://www.chnspec.net/What-is-the-Lab-Color-Space-LCh.html#:~:text=Lab:%20Uses%20Cartesian%20coordinates%20to,%2C%20and%20Hue%20(h)
            auto pxValue = std::sqrt((std::pow(*labAPx, 2.0) + std::pow(*labBPx, 2.0)));
            outPx[columnIndex] = pxValue; 
        }
    }
    return outMask;
}

cv::Mat AbsColourDistance::ColourDetector::getLCHuvHUEMat(const cv::Mat& labAChannel, const cv::Mat& labBChannel, const ChannelType channelType)
{
    if (channelType != ChannelType::LCHuv_HUE)
    {
        ErrorOutput(Errors::BrickCVErrors::WRONG_COLOUR_CHANNEL, "getLCHuvHUEMat() called for channel of type", getChannelName(channelType));
        return cv::Mat();
    }

    if (labAChannel.rows != labBChannel.rows || labBChannel.cols != labAChannel.cols)
    {
        Errors::ErrorOutput(Errors::IMAGEs_NOT_EQUAL_SIZE, "Input for this function has to have equal rows and columns but didn't.");
        return cv::Mat();
    }

    cv::Mat outMask = cv::Mat::zeros(labAChannel.rows, labBChannel.cols, CV_64F);

    auto rows = labAChannel.rows;
    auto cols = labBChannel.channels() * labBChannel.cols;

    auto bothContinuous = labAChannel.isContinuous() && labBChannel.isContinuous();
    if (bothContinuous)
    {
        cols = cols * rows;
        rows = 1;
    }

    //convert to suitable datatype
    cv::Mat labA;
    cv::Mat labB;
    labAChannel.convertTo(labA, CV_32F);
    labBChannel.convertTo(labB, CV_32F);

    //loop over all pixels in the images and apply transform to outpu
    for (auto rowIndex = 0; rowIndex < rows; rowIndex++)
    {
        auto labAPx = labA.ptr<float>(rowIndex);
        auto labBPx = labB.ptr<float>(rowIndex);
        auto outPx = outMask.ptr<float>(rowIndex);

        for (auto columnIndex = 0; columnIndex < cols; columnIndex++)
        {
            // Hue -> h = arctan(b/a) * (180/pi) - https://www.chnspec.net/What-is-the-Lab-Color-Space-LCh.html#:~:text=Lab:%20Uses%20Cartesian%20coordinates%20to,%2C%20and%20Hue%20(h)
            // This means the pixel value will represent an angle - https://www.chnspec.net/What-is-the-Lab-Color-Space-LCh.html#:~:text=Lab:%20Uses%20Cartesian%20coordinates%20to,%2C%20and%20Hue%20(h).
            auto pxValue = std::atan((*labBPx / *labAPx)) * (180.0 / sc_pi_approx);
            outPx[columnIndex] = pxValue;
        }
    }
    return outMask;
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
