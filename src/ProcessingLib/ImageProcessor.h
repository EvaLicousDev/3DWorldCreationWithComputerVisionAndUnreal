#pragma once 

//standard library includes
#include <vector>
#include <memory>

//Open CV includes 
#include <opencv2/imgproc.hpp>
#include <opencv2/core/core.hpp>

namespace PreProcessor
{
    using imagePtr = std::shared_ptr<cv::Mat>;

    static constexpr int sc_coloursToIdentify = 8;

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
            std::vector<cv::Mat> rgb_channels;
            cv::split(in_image, rgb_channels);
            image_R = rgb_channels[0];
            image_G = rgb_channels[1];
            image_B = rgb_channels[2];

            //seperate and retrieve hsv colour channels
            cv::Mat hsv; 
            cv::cvtColor(in_image, hsv, cv::COLOR_RGB2HSV);
            std::vector<cv::Mat> hsv_channels;
            cv::split(hsv, hsv_channels);
            image_hue = hsv_channels[0];
            image_saturation = hsv_channels[1];
            image_value = hsv_channels[2]; 

            //seperate and retrieve lap colour channels
            cv::Mat lab;
            cv::cvtColor(in_image, lab, cv::COLOR_RGB2Lab);
            std::vector<cv::Mat> lab_channels;
            cv::split(lab, lab_channels);
            image_Lumosity = lab_channels[0];
            image_Axis = lab_channels[1];
            image_BlueYellow = lab_channels[2];

            //seperate and retrieve luv colour channels
            cv::Mat luv;
            cv::cvtColor(in_image, luv, cv::COLOR_RGB2Luv);
            std::vector<cv::Mat> luv_channels;
            cv::split(luv, luv_channels);
            image_Lumosity2 = luv_channels[0];
            image_U = luv_channels[1];
            image_V = luv_channels[2];
        }

        //image processing functions using pixle wise operations
        cv::Mat naiveRgbColourSpaceReduction(cv::Mat& image,   int divideBy = 16);
        cv::Mat naiveRgbColourSpaceReduction2(cv::Mat& image,  int divideBy = 16);
        cv::Mat bitwiseRgbColourSpaceReduction(cv::Mat& image, int divideBy = 16);
        cv::Mat rgbColourSpaceReductionWithIt(cv::Mat& image,  int divideBy = 16);

        void updateHSVChannelsWithProcessed(cv::Mat& processed); 

        cv::Mat sharpen2Dedges(cv::Mat& image); 
        int     getDistanceToTargetColour(const cv::Vec3b& colourIn, const cv::Vec3b& tragetColour) const; 

        //debug functions for development
        cv::Mat& getMainImage() { return in_image; }

        void     debugInfo(cv::Mat& image);

        void     display(cv::Mat& image, bool sizeDown = true);
        void     display(cv::Mat& image,  int pixels);
        void     displayRGBChannels();
        void     displayHSVChannels(); 
        void     displayHSVChannelsAndOriginal(); 
        void     displayLABChannels();
        void     displayLUVChannels();

    private:
        struct HistogramData
        {
            
        };

        cv::Mat in_image; 

        //RGB channels
        cv::Mat image_R; 
        cv::Mat image_G; 
        cv::Mat image_B; 

        //HSV channels 
        cv::Mat image_hue; 
        cv::Mat image_saturation; 
        cv::Mat image_value; 

        //LAB channels
        cv::Mat image_Lumosity; 
        cv::Mat image_Axis; 
        cv::Mat image_BlueYellow; 

        //Luv channels
        cv::Mat image_Lumosity2;
        cv::Mat image_U;
        cv::Mat image_V;

        //cv::Mat createColourLocationImageBW(int maxDistance, const cv::Vec3b& tragetColour);

    };
}
