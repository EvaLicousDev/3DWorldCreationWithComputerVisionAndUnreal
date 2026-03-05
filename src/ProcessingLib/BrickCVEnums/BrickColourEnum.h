#pragma once

namespace BrickCV
{
    enum BrickColour : int8_t
    {
        PURPLE = 0,
        DARK_BLUE = 1,
        LIGHT_BLUE = 2,
        BROWN = 3,
        PINK = 4,
        LIGHT_GREEN = 5,
        DARK_GREEN = 6,
        RED = 7,
        ORANGE = 8,
        YELLOW = 9,
        WHITE = 10,
        BLACK = 11
    };


    static const constexpr char* getBrickColour(const BrickColour& brickColour)
    {
        switch (brickColour)
        {
        case DARK_GREEN:
            return "Dark Green";
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
        case LIGHT_BLUE:
            return "Light Blue";
            break;
        case LIGHT_GREEN:
            return "Light Green";
            break;
        case BROWN:
            return "Brown";
            break;
        case BLACK:
            return "Black";
            break;
        default:
            return "sth went really wrong with determaning the brick colour if you see this.";
        }
    }

    static const constexpr BrickColour sc_coloursInUse[] = { PURPLE, DARK_BLUE, LIGHT_BLUE, LIGHT_GREEN, BROWN, RED, ORANGE, YELLOW, DARK_GREEN };
}
