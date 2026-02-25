#pragma once
#include <vector>
#include <opencv2/imgcodecs.hpp>

#include "ERRORs/ErrorOutput.h"
#include "Histogram/ColourHistogram.h"

namespace BrickCv
{
    static const int8_t sc_luminanceChannelID = 0; 
    static const int8_t sc_axisChannelID = 1;
    static const int8_t sc_blueYellowChannelID = 2;

    class BrickColourClassifier
    {
        static const constexpr char* brickSampleImagePath = "BrickExampleColourImages"; 

    public:

        // This class will upon construction create all colour histograms for colours we want to detect from example images
        // It then contains functions to threshold and seperate out various colours to establish classification certainty
        BrickColourClassifier()
        {
            for (auto colour : BrickCV::sc_coloursInUse)
            {
                m_colourPosition.push_back(colour);
                const char* colourExamplePath = BrickCV::getHistogramImageAdress(colour); 
                if (std::strcmp(colourExamplePath, "") != 0) //make sure string not empty
                {
                    //load the example image and split into it's channels
                    cv::Mat labExample;
                    cv::Mat matChannels[3];
                    cv::Mat example = imread(colourExamplePath, cv::IMREAD_COLOR_BGR);
                    cv::cvtColor(example, labExample, cv::COLOR_BGR2Lab);
                    cv::split(labExample, matChannels);

                    //adjust light channel value range from 0-100 to 0-255 like A & B channels
                    cv::normalize(matChannels[sc_luminanceChannelID], matChannels[sc_luminanceChannelID], 0, 255);

                    //blur image with median as we want to even colour out 
                    cv::medianBlur(matChannels[sc_luminanceChannelID], matChannels[sc_luminanceChannelID], 9);
                    cv::medianBlur(matChannels[sc_axisChannelID], matChannels[sc_axisChannelID], 9);
                    cv::medianBlur(matChannels[sc_blueYellowChannelID], matChannels[sc_blueYellowChannelID], 9);
                }
                else
                {
                    Errors::ErrorOutput(Errors::BrickCVErrors::IMAGE_PATH_EMPTY, "IMPORTANT! Histogram path could not be loaded for colour ", BrickCV::getBrickColour(colour)); 
                }
            }
        }

        ~BrickColourClassifier()
        {
            m_colourPosition.clear();
            m_luminanceChannel.clear();
            m_axisChannel.clear();
            m_blueYellowChannel.clear(); 
        }

    private:
        std::vector<BrickCV::BrickColour>       m_colourPosition;
        std::vector<Histogram::ColourHistogram> m_luminanceChannel; 
        std::vector<Histogram::ColourHistogram> m_axisChannel;
        std::vector<Histogram::ColourHistogram> m_blueYellowChannel;
    };
}
