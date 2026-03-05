#pragma once
#include <opencv2/imgproc.hpp>

#include "PredefinedColours.h"
#include "BrickCVEnums/BrickColourEnum.h"
#include "BrickCVEnums/ChannelType.h"

using namespace BrickCV;
namespace AbsColourDistance
{
    // using HTML colours https://www.w3schools.com/tags/ref_colornames.asp & https://www.computerhope.com/htmcolor.htm#color-codes
    // colour space is RGB888 as camera also uses RGB888
    class ColourDetector
    {
        //All colours are stored as 24 bit hexadecimal number combination in a 32 bit integer
        const std::vector<uint32_t> purple     = { RGB888::HotPink, RGB888::Purple, RGB888::Lavender, RGB888::Plum, RGB888::Violet, RGB888::Orchid, RGB888::DarkViolet, RGB888::DarkOrchid, RGB888::Indigo, RGB888::DarkSlateBlue, RGB888::SlateBlue };
        const std::vector<uint32_t> darkBlue   = { RGB888::DarkBlue, RGB888::Navy, RGB888::MidnightBlue, RGB888::MediumBlue, RGB888::RoyalBlue, RGB888::Blue, RGB888::SteelBlue};
        const std::vector<uint32_t> lightBlue  = { RGB888::LightCyan, RGB888::PaleTurquoise, RGB888::CadetBlue, RGB888::Teal, RGB888::LightSteelBlue, RGB888::LightSkyBlue, RGB888::SkyBlue };
        const std::vector<uint32_t> brown      = { RGB888::Cornsilk, RGB888::BlanchedAlmond, RGB888::Tan, RGB888::SandyBrown, RGB888::Peru, RGB888::Chocolate, RGB888::SaddleBrown, RGB888::Sienna, RGB888::Brown, RGB888::Maroon};
        const std::vector<uint32_t> pink       = { RGB888::Pink, RGB888::LightPink, RGB888::DeepPink, RGB888::PaleVioletRed, RGB888::MediumVioletRed };
        const std::vector<uint32_t> lightGreen = { RGB888::LimeGreen, RGB888::YellowGreen, RGB888::Lime, RGB888::GreenYellow, RGB888::PaleGreen, };
        const std::vector<uint32_t> darkGreen  = { RGB888::DarkOliveGreen, RGB888::Olive, RGB888::OliveDrab, RGB888::LawnGreen, RGB888::SpringGreen, RGB888::MediumSpringGreen, RGB888::MediumSeaGreen, RGB888::Green, RGB888::DarkGreen};
        const std::vector<uint32_t> red        = { RGB888::LightSalmon, RGB888::Salmon, RGB888::DarkSalmon, RGB888::LightCoral, RGB888::IndianRed, RGB888::Crimson, RGB888::Firebrick, RGB888::DarkRed, RGB888::Red};
        const std::vector<uint32_t> orange     = { RGB888::OrangeRed, RGB888::Tomato, RGB888::Coral, RGB888::DarkOrange, RGB888::Orange};
        const std::vector<uint32_t> yellow     = { RGB888::Yellow, RGB888::LemonChiffon, RGB888::LightGoldenrodYellow, RGB888::PapayaWhip, RGB888::Moccasin, RGB888::PeachPuff, RGB888::PaleGoldenrod, RGB888::Khaki, RGB888::DarkKhaki, RGB888::Gold};
        const std::vector<uint32_t> white      = { RGB888::White, RGB888::Snow, RGB888::Honeydew, RGB888::MintCream, RGB888::Azure, RGB888::GhostWhite, RGB888::Seashell, RGB888::Ivory, RGB888::AntiqueWhite, RGB888::Linen, RGB888::LightGray };

        const std::vector<std::vector<uint32_t>> exampleBrickShades = { purple, darkBlue, lightBlue, brown, pink, lightGreen, darkGreen, red, orange, yellow, white };
        const std::vector<BrickCV::BrickColour>  vectorNames        = {    PURPLE,   DARK_BLUE,   LIGHT_BLUE,   BROWN,   PINK,   LIGHT_GREEN,   DARK_GREEN,   RED,   ORANGE,    YELLOW,    WHITE };

        static const cv::Scalar getBGRColour(const uint32_t colour)
        {
            //uses bitshifting and bitmasking to extract integer values into RGB format
            uint8_t red   = (colour >> 16) & 0xFF; 
            uint8_t green = (colour >> 8) & 0xFF;
            uint8_t blue  = (colour) & 0xFF;

            //note we use BGR as native format to open cv, so we swap channel positions for blue and red
            return cv::Scalar(blue, green, red);
        }

    public:
        ColourDetector() = default;  
        static cv::Mat processChannel(const cv::Mat image, const cv::Mat& example, int& boundry, BrickCV::ChannelType channelType);
        const BrickColour getBrickApproximation(const cv::Mat& colourSample); 
    };
}
