#pragma once

namespace BrickCV
{
    enum ChannelType : int8_t
    {
        NOT_DECLARED = 0,
        BGR_RED = 1,
        BGR_GREEN = 2,
        BGR_BLUE= 3,
        HSV_HUE = 4,
        HSV_SATURATION = 5,
        HSV_VALUE = 6,
        LAB_LUMINANCE = 7,
        LAB_AXIS = 8,
        LAB_BLUE_YELLOW = 9
    };

    static const constexpr char* getChannelName(const ChannelType& channel)
    {
        switch (channel)
        {
        case NOT_DECLARED:
            return "NOT DECLARED";
            break;
        case BGR_RED:
            return "Red";
            break;
        case BGR_GREEN:
            return "Green";
            break;
        case BGR_BLUE:
            return "Blue";
            break;
        case HSV_HUE:
            return "Hue";
            break;
        case HSV_SATURATION:
            return "Saturation";
            break;
        case HSV_VALUE:
            return "Colour";
            break;
        case LAB_LUMINANCE:
            return "Luminance";
            break;
        case LAB_AXIS:
            return "Axis";
            break;
        case LAB_BLUE_YELLOW:
            return "BlueYellow";
            break;
        default:
            return "sth went really wrong if you see this.";
        }
    }
}