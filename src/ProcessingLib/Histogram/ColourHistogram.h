#pragma once
#include "SingleChannelHistogram.h"
#include "../BrickCVEnums/BrickColourEnum.h"
#include "../BrickCVEnums/ChannelType.h"

namespace Histogram
{
    class ColourHistogram
    {
        //This class is designed to contain and calculate histograms for individual brick colours specifically
        //Theory: Histogram back projection returns a likelyhood map which allows us to tell how likely a pixel is to belong to a certain colour.
        //        This information can then be thresholded to maka judgement of what the most amount of pixles in an area blong to

    public: 
        ColourHistogram(BrickCV::BrickColour brickColour, BrickCV::ChannelType channel, cv::Mat& channelMat);
        ~ColourHistogram()
        {
            histogramPtr.reset();
        }

        std::weak_ptr<SingleChannelHistogram> getHistogram(){ return histogramPtr; }

    private:
        std::shared_ptr<SingleChannelHistogram> histogramPtr; 

        BrickCV::ChannelType channel     = BrickCV::ChannelType::NOT_DECLARED;
        BrickCV::BrickColour brickColour = BrickCV::BrickColour::BLACK; 
    };
}
