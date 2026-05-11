#include "ColourHistogram.h"

#include "opencv2/imgproc.hpp"

#include "../BrickCVEnums/BrickColourEnum.h"
#include "../BrickCVEnums/ChannelType.h"
#include "../ERRORs/ErrorOutput.h"

Histogram::ColourHistogram::ColourHistogram(BrickCV::BrickColour brickColour, BrickCV::ChannelType channel, cv::Mat& channelMat)
{
    this->channel     = channel;
    this->brickColour = brickColour;

    //Take single channel image and create pointer to Histogram
    if (channelMat.channels() == 1)
    {
        this->histogramPtr = std::make_shared<SingleChannelHistogram>(channelMat);
        if (this->histogramPtr == nullptr)
        {
            Errors::ErrorOutput(Errors::COULD_NOT_ALLOCATE,
                "This ColourHistogram instance with colour ",
                BrickCV::getBrickColour(brickColour),
                " and channel ",
                BrickCV::getChannelName(channel),
                " could not allocate the memory for the histogram.");
        }
    }
    else
    {
        Errors::ErrorOutput(Errors::BrickCVErrors::NOT_SINGLE_CHANNEL_IMAGE,
            "This ColourHistogram instance with colour ", 
            BrickCV::getBrickColour(brickColour), 
            " and channel ", 
            BrickCV::getChannelName(channel), 
            " was initiated wrong."); 
    }
}
