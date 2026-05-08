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

    // strictly has to be in order of enum above!
    // all colours except black as black ~should~ be easy to identify through thresholding so we don't need to train a ML model
    static const constexpr char* sc_trainingImageFolderPaths[11] = { "../TrainingImagesFolder/Purple/*.jpg",
                                                                  "../TrainingImagesFolder/DarkBlue/*.jpg",
                                                                  "../TrainingImagesFolder/LightBlue/*.jpg",
                                                                  "../TrainingImagesFolder/Brown/*.jpg",
                                                                  "../TrainingImagesFolder/Pink/*.jpg",
                                                                  "../TrainingImagesFolder/LightGreen/*.jpg",
                                                                  "../TrainingImagesFolder/DarkGreen/*.jpg",
                                                                  "../TrainingImagesFolder/Red/*.jpg",
                                                                  "../TrainingImagesFolder/Orange/*.jpg",
                                                                  "../TrainingImagesFolder/Yellow/*.jpg",
                                                                  "../TrainingImagesFolder/White/*.jpg"
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

    //this array should contain the colours available in the top area or lego plate in order left to right, not containing the base colour
    static const constexpr BrickColour sc_coloursInUse[] = { WHITE, PURPLE, DARK_BLUE, LIGHT_BLUE, BROWN, RED, YELLOW, LIGHT_GREEN };
}
