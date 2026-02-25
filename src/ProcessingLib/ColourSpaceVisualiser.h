#pragma once

//standard library includes
#include <vector>
#include <memory>

//templates
#include "TemplateCode/CommonOperations.h"

//Open CV includes 
#include <opencv2/imgproc.hpp>
#include <opencv2/core/core.hpp>

#include "ERRORs/ErrorOutput.h"

namespace PreProcessor
{
    using imagePtr = std::shared_ptr<cv::Mat>;

    class ColourSpaceVisualiser
    {
        /*
         * The purpose of this class is to visualisation of the following colour spaces for a given image
         *  - BGR -> note this application loads images as BGR as that is the native format of opencv, this class stores channel adresses in RGB order however
         *  - HSV
         *  - LAB
         *  - lUV
         *
         *  This class is primarely used in the development process and for debugging image processing. 
         */

    public: 
        static constexpr int sc_channelsToVisualise = 14; 

        ColourSpaceVisualiser(imagePtr loadedImage)
            : in_image(*loadedImage)
        {
            Errors::ErrorOutput(Errors::BrickCVErrors::NULL_PTR, "ColourSpaceVisualiser initialised with nullptr, please investigate!");

            std::vector<cv::Mat> bgr_channels;
            cv::split(in_image, bgr_channels);
            image_R = bgr_channels[2];
            image_G = bgr_channels[1];
            image_B = bgr_channels[0];

            //seperate and retrieve hsv colour channels
            cv::Mat hsv;
            cv::cvtColor(in_image, hsv, cv::COLOR_BGR2HSV);
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

            cv::cvtColor(in_image, image_greyscale, cv::COLOR_BGR2GRAY);

        }

        ~ColourSpaceVisualiser()
        {
            for (cv::Mat* channel : channelAdresses)
            {
                channel->release(); 
            }
        }

        void updateHSVChannelsWithProcessed(cv::Mat& processed);

        static void display(const cv::Mat& image, const char* frameName, bool sizeDown = true);
        void     display(cv::Mat& image, const char* frameName, int pixels);
        void     displayRGBChannels();
        void     displayHSVChannels();
     //   void     displayHSVChannelsAndOriginal();
        void     displayLABChannels();
        void     displayLUVChannels();

        cv::Mat  getLuminance() { return image_Lumosity; }
        cv::Mat  getAxis() { return image_Axis; }
        cv::Mat  getBY() { return image_BlueYellow; }
        cv::Mat  getRed() { return image_R; }
        cv::Mat  getSaturation() { return image_saturation; }

    private:

        cv::Mat in_image;

        //RGB/BGR channels
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

        cv::Mat image_greyscale;
        const std::array<cv::Mat*, 14> channelAdresses = {&in_image, &image_R, &image_G, &image_B, &image_hue, &image_saturation, &image_value, &image_Lumosity, &image_Axis, &image_BlueYellow, &image_Lumosity2, &image_U, &image_V}; 
    };
}
