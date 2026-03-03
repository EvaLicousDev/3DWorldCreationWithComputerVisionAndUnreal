#pragma once

namespace BrickCV
{
    enum BrickColour : int8_t
    {
        NOT_SET = 0,
        RED = 1,
        YELLOW = 2,
        ORANGE = 4,
        PURPLE = 5,
        DARK_BLUE = 6,
        PINK = 7,
        WHITE = 8
    };


    static const constexpr char* getBrickColour(const BrickColour& brickColour)
    {
        switch (brickColour)
        {
        case NOT_SET:
            return "NOT DECLARED";
            break;
        case RED:
            return "Red";
            break;
        case YELLOW:
            return "Yellow";
            break;
        case DARK_BLUE:
            return "Blue";
            break;
        case ORANGE:
            return "Orange";
            break;
        case PURPLE:
            return "Purple";
            break;
        case PINK:
            return "Pink";
            break;
        case WHITE:
            return "White";
            break;
        default:
            return "sth went really wrong with determaning the brick colour if you see this.";
        }
    }

    static const constexpr BrickColour sc_coloursInUse[] = { RED, ORANGE };
    static const constexpr char* getHistogramImageAdress(const BrickColour& brickColour)
    {
        switch (brickColour)
        {
        case NOT_SET:
            return "";
            break;
        case RED:
            return "BrickExampleColourImages/RED_HistogramTestImage.jpg";
            break;
        case YELLOW:
            return "";
            break;
        case DARK_BLUE:
            return "";
            break;
        case ORANGE:
            return "BrickExampleColourImages/ORANGE_HistogramTestImage.jpg";
            break;
        case PURPLE:
            return "";
            break;
        case PINK:
            return "";
            break;
        case WHITE:
            return "";
            break;
        default:
            return "";
        }
    }
}
