#pragma once 

//standard library includes
#include <vector>
#include <memory>

//Open CV includes 
#include <opencv2/core/core.hpp>

namespace PreProcessor
{
    using imagePtr = std::shared_ptr<cv::Mat>;

    class ImageProcessor
    {
        /*
         * This class was created to encapsulate the preprocessing functions needed for detection of lego bricks in the images
         * provided by the camera.
         *
         * Goal:
         * Find the locations of different colour lego pieces in the image.
         *
         * Theory:
         * The RGB space provides 256 x 256 x 256 colours over three channels (approximately 16 mil).
         * Therefor we want to reduce the number of colours processed down to the minimum needed. For this we
         * divide the RGB space into "X" components where "X" stands for the number of configurable colours
         * for the Lego map we are trying to analyse.
         *
         * Approach:
         * We first apply some processing to identify the position of the boards from the lego bricks.
         * Then we isolate a mask for an area of interest over the "example plate" to apply some thresholding for colour values. 
         * This way we can dynamically set the colour ranges, allowing for slightly more resiliance to bad lighting. 
         * Note that colours are chosen in a way that allows for high contrast during processing. The AOI mask for example uses 
         * a transition from white to black to identify the AOI. 
         */

    public:
        ImageProcessor(const cv::Mat& loadedImage)
            : in_image(loadedImage)
        {
        }

        //image processing functions using pixle wise operations
        cv::Mat naiveRgbColourSpaceReduction(cv::Mat& image, int divideBy = 16);
        cv::Mat naiveRgbColourSpaceReduction2(cv::Mat& image, int divideBy = 16);
        cv::Mat bitwiseRgbColourSpaceReduction(cv::Mat& image, int divideBy = 16);
        cv::Mat rgbColourSpaceReductionWithIt(cv::Mat& image, int divideBy = 16);
        cv::Mat sharpen2Dedges(cv::Mat& image); 

        //basic class functions 
        cv::Mat& getMainImage() { return in_image; }
        void     debugInfo(cv::Mat& image);
        void     display(int widthPixels, int heightPixels);
        void     display(cv::Mat& image, int pixels = 500);

    private:
        cv::Mat in_image; 
    };
}
