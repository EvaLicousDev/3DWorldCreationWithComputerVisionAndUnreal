#pragma once
#include <opencv2/core/mat.hpp>

#include "opencv2/imgproc.hpp"

namespace Histogram
{
    struct GraphCreator
    {
        //The implemenatation is based of OpenCV 4 Computer Vision Applcation Cookbook 
        //draws vertical lines corresponding to the amount of pixels with the specific value of that bin [0-255]
        static cv::Mat createHistorgramGraph(cv::Mat& histogram)
        {
            double minVal;
            double maxVal;
            cv::minMaxLoc(histogram, &minVal, &maxVal, 0, 0);

            auto histImage = cv::Mat(histogram.rows, histogram.rows, CV_8UC1, cv::Scalar(255));
            int highest = static_cast<int>(90 * histogram.rows);

            for (int h = 0; h < histogram.rows; h++)
            {
                float binVal = histogram.at<float>(h);
                if (binVal > 0)
                {
                    int intensity = static_cast<int>(binVal * highest / maxVal);
                    cv::line(histImage, cv::Point(h, histogram.rows), cv::Point(h, histogram.rows - intensity), cv::Scalar(0), 1);
                }
            }

            return histImage;
        }
    };
}
