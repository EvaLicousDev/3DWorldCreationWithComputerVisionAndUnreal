#pragma once

#include "../Histogram/SingleChannelHistogram.h"
#include "../BrickCVEnums/BrickColourEnum.h"
#include "../BrickCVEnums/ChannelType.h"

#include <vector>
#include <memory>

//Open CV includes 
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

namespace brickColourSVM
{
    static const int aChannelIndex = 1; 
    static const int bChannelIndex = 2; 

    static const int blueChannelIndex  = 0;
    static const int greenChannelIndex = 1;
    static const int redChannelIndex   = 2;

    struct UnprocessedTrainingDataInstance
    {
        UnprocessedTrainingDataInstance() = default; 
        UnprocessedTrainingDataInstance(const cv::String path, BrickCV::BrickColour colour)
        {
            //get image
            cv::Mat imageToProcess = cv::imread(path, cv::ImreadModes::IMREAD_COLOR_BGR);

            //get mean values for colour channels
            cv::Scalar meanValues = mean(imageToProcess);
            bgrMeanValues = std::make_unique<cv::Scalar>(meanValues); 

            //get Lab image information
            cv::cvtColor(imageToProcess, imageToProcess, cv::COLOR_BGR2Lab);
            cv::Mat labChannels[3];
            cv::split(imageToProcess, labChannels);

            //Create normalised histograms for A & B channel of Lab image
            auto aChannelHist = Histogram::SingleChannelHistogram(labChannels[aChannelIndex]);
            auto bChannelHist = Histogram::SingleChannelHistogram(labChannels[bChannelIndex]);
            histogramLAB_Achannel = std::make_shared<Histogram::SingleChannelHistogram>(aChannelHist);
            histogramLAB_Bchannel = std::make_shared<Histogram::SingleChannelHistogram>(bChannelHist);
        }

        bool isValid() { return histogramLAB_Achannel != nullptr && histogramLAB_Bchannel != nullptr && bgrMeanValues != nullptr; }

        std::string imagePath = "";
        BrickCV::BrickColour brickColour = BrickCV::BrickColour::BLACK; 
        std::shared_ptr<cv::Scalar> bgrMeanValues = nullptr;
        std::shared_ptr<Histogram::SingleChannelHistogram> histogramLAB_Achannel = nullptr; 
        std::shared_ptr<Histogram::SingleChannelHistogram> histogramLAB_Bchannel = nullptr;
    };
}