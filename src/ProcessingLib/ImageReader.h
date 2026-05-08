#pragma once

#include "ERRORs/ErrorOutput.h"

#include <memory>
#include <opencv2/core.hpp>

using namespace Errors; 
namespace ImageProcessing
{
    class ImageReader
    {
        /*
        * Class that manages the loading and lifetime of the images provided in the Image folder in memory
        */

    public:
        ImageReader() = default;
        ~ImageReader() 
        {
           for (auto imagPtr : images)
           {
               imagPtr.reset(); 
           }
        };

        std::vector<std::weak_ptr<cv::Mat>> getImages(); 
        void readImages(const char* imagePath); 
        std::vector<std::string> fileNames{};

    private:
        std::vector<std::shared_ptr<cv::Mat>> images{}; 
    };
}