#pragma once
#include <opencv2/core/mat.hpp>

namespace brickHDR
{
    class CustomHDRImageArrayMerger
    {
    // Theory: If you know the exposure time an image was taken with and the cameras exposure function you can create custom HDR images if you have at least seperate images
    // This code is based of: https://docs.opencv.org/3.4/d3/db7/tutorial_hdr_imaging.html

    public:
        CustomHDRImageArrayMerger(const char* imagesFolderPath)
            : m_imagesPath(imagesFolderPath)
        {}

        cv::Mat mergeImages(); 

    private:
        const char* m_imagesPath; 
        void loadloadExposureSequence(const char* path, std::vector<cv::Mat>& images, std::vector<float>& times);
    };
}
