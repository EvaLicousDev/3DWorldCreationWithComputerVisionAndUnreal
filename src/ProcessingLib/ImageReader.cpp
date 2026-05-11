#include "ImageReader.h"

#include "ERRORs/ErrorOutput.h"
#include "ERRORs/ErrorTypes.h"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <filesystem>
#include <thread>
#include <iostream>

namespace filesystem = std::filesystem; 

std::vector<std::weak_ptr<cv::Mat>> ImageProcessing::ImageReader::getImages()
{
    if (m_images.empty())
    {
        Errors::ErrorOutput(Errors::NULL_PTR, "Image Reader was prompted to return images, but image vector is empty. Did the timer run out?");
        throw std::exception("No images emplaced in vector");
    }

    std::vector<std::weak_ptr<cv::Mat>> outvector{}; 
    for (const auto image : m_images)
    {
        outvector.emplace_back(image); 
    }

    return outvector;
}

void ImageProcessing::ImageReader::readImages(const char* imagesPath)
{
    auto timerStart = std::chrono::high_resolution_clock().now(); 

    while (m_fileNames.empty())
    {
        //get all filenames for a specified pattern 
        if (filesystem::exists(filesystem::path(imagesPath)))
        {
            std::cout << "Found the filepath!!" << std::endl; 
            cv::String folderpath = imagesPath;
            cv::glob(folderpath, m_fileNames, false);

            for (auto file : m_fileNames)
            {
                auto image = cv::imread(file, cv::IMREAD_COLOR);
                auto pointer = std::make_shared<cv::Mat>(image);
                this->m_images.emplace_back(pointer);
            }
        }

        auto timerCheck = std::chrono::high_resolution_clock().now();
        auto timePassed = std::chrono::duration_cast<std::chrono::seconds>(timerCheck - timerStart); 
        if (timePassed.count() > 0 && (timePassed.count() % 20) == 0)
        {
            std::cout << "Still trying..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1)); 
        }

        if (timePassed.count() > 500)
        {
            std::cout << "[IMPORTANT INFORMATION] \t 5 minutes have passed and the images have not been found at the specified location. \t \t \t \t \t The application will now be terminated." << std::endl; 
            return;
        }
    }
}
