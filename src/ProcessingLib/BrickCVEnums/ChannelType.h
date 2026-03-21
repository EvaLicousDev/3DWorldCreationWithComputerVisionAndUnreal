#pragma once

// To accomodate LCHuv addtional logic would need to be implemented. MatLab offers logic to convert RGB to LCH images 
// https://uk.mathworks.com/matlabcentral/fileexchange/100943-rgb_lch_conversion?s_tid=FX_rc2_behav

namespace BrickCV
{
    enum ChannelType : int8_t
    {
        NOT_DECLARED = 0,
        BGR_RED = 1,                // BGR is used over RGB due to BGR are being the native format to openCV according to the literature review
        BGR_GREEN = 2,
        BGR_BLUE = 3,
        HSV_HUE = 4,                // HSV is not used in the final implementation as testing has prooven for it to be to unreliable when 
        HSV_SATURATION = 5,         // presented with varying lighting conditions, reflective surfaces of lego, and different camera qualities. 
        HSV_VALUE = 6,              // Therefor this is only here for completeness. Most code using HSV has been removed, other than in the ColourSpaceVisualiser
        LAB_LUMINANCE = 7,
        LAB_AXIS = 8,               // LAB delivered best results in testing with varying camera qualities, lighting & reflective surfaces
        LAB_BLUE_YELLOW = 9,
        LUV_LUMINANCE = 10,   
        LUV_UNIFORM_REDGREEN = 11,  // LUV delivered similarly results to LAB and has the advantage of being perceptually uniform. 
        LUV_V_BLUEYELLOW = 12,      // The decision to use LAB over LUV primarely was based on literature review. 
        LCHuv_LUMINANCE = 14,       // LCHuv is a cylindrical colour model combining the strengths and intuitive features of HSV but 
        LCHuv_CHROMA = 15,          // is supposed to remain more accurate in terms of preserving Hue under varying lighting conditions.
        LCHuv_HUE = 16              // This was added as a possible extension to the project after some more further reading
    };

    enum ColourSpace : int8_t
    {
        BGR = 0,
        HSV = 1,
        LAB = 2,
        LUV = 4,
        LCHuv = 5
    };

    static const constexpr char* getChannelName(const ChannelType& channel)
    {
        switch (channel)
        {
        case NOT_DECLARED:
            return "NOT DECLARED";
            break;
        case BGR_RED:
            return "BGR Red";
            break;
        case BGR_GREEN:
            return "BGR Green";
            break;
        case BGR_BLUE:
            return "BGR Blue";
            break;
        case HSV_HUE:
            return "HSV Hue";
            break;
        case HSV_SATURATION:
            return "HSV Saturation";
            break;
        case HSV_VALUE:
            return "HSV Value";
            break;
        case LAB_LUMINANCE:
            return "LAB Luminance";
            break;
        case LAB_AXIS:
            return "LAB Axis";
            break;
        case LAB_BLUE_YELLOW:
            return "LAB BlueYellow";
            break;
        case LUV_LUMINANCE:
            return "Lu\'v\' Lightness";
            break; 
        case LUV_UNIFORM_REDGREEN:
            return "Lu\'v\' RedGreen";
            break;
        case LUV_V_BLUEYELLOW:
            return "Lu\'v\' BlueYellow";
            break;
        case LCHuv_LUMINANCE:
            return "LCHu\'v\' Lightness";
            break;
        case LCHuv_CHROMA:
            return "LCHu\'v\' Chromaticity";
            break;
        case LCHuv_HUE:
            return "LCHu\'v\' Hue";
            break;
        default:
            return "Whoopsydaisy :( this colour channel is undefinened somehow. Check ChannelType Enum vs getChannelNam() function!";
        }
    }
}