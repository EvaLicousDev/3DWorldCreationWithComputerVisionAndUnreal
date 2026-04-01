#include "ImageReader.h"

#include "ERRORs/ErrorOutput.h"
#include "ERRORs/ErrorTypes.h"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

std::vector<std::weak_ptr<cv::Mat>> ImageProcessing::ImageReader::getImages()
{
    if (images.empty())
    {
        Errors::ErrorOutput(Errors::NULL_PTR, "Image Reader was prompted to return images, but image vector is empty");
        throw std::exception("No images emplaced in vector");
    }

    std::vector<std::weak_ptr<cv::Mat>> outvector{}; 
    for (const auto image : images)
    {
        outvector.emplace_back(image); 
    }

    return outvector;
}

void ImageProcessing::ImageReader::readImages(const char* imagesPath)
{
    //get all filenames for a specified pattern 
    cv::String folderpath = imagesPath; 
    std::vector<cv::String> filepaths; 
    cv::glob(folderpath, filepaths, false);

    if (filepaths.empty()) //break if we couldn't find images
    {
        Errors::ErrorOutput(Errors::BrickCVErrors::NULL_PTR, "Reading files for filepath \'", imagesPath, "\' was unsuccessful. Please check accuracy of path");
        throw std::exception("No files found for specifiied filepath");
    }

    for (auto file : filepaths)
    {
        auto image = cv::imread(file, cv::IMREAD_COLOR); 
        auto pointer = std::make_shared<cv::Mat>(image);
        this->images.emplace_back(pointer); 
    }
}
