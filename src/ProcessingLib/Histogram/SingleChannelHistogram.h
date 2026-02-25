#pragma once

//standard library includes
#include <vector>
#include <memory>

//Open CV includes 
#include <opencv2/imgproc.hpp>
#include <opencv2/core/core.hpp>

namespace Histogram
{
    class SingleChannelHistogram
    {
        //This class is specifically designed to create a simpler interface for one dimensional histogram using Open CV library functions
    public: 
        SingleChannelHistogram(cv::Mat& image1D)
        {
            cv::Mat images[1] = {image1D}; 
            cv::calcHist(images, 1, channel, cv::Mat(), m_histogram, 1, histogramSections, range, true, false);
        }

        ~SingleChannelHistogram()
        {
            m_histogram.release();
        }

        cv::Mat getHistogram() { return m_histogram; }

    private:
        //default arguments amount to greyscale image with range 0 - 255 pixel values
        int histogramSections[1] = { 256 };
        float hranges[2] = {0.0, 255.0};
        const float* range[1] = { hranges };
        int channel[1] = {0};

        cv::Mat m_histogram; 
    };
}