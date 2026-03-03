#pragma once
#include <opencv2/imgproc.hpp>

#include "BrickCVEnums/ChannelType.h"

namespace AbsColourDistance
{
    class ColourDetector
    {
    public:
        static cv::Mat processChannel(const cv::Mat image, const cv::Mat& example, int& boundry, BrickCV::ChannelType channelType);
    };
}
