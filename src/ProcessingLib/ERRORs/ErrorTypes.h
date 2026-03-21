#pragma once

namespace Errors
{
    enum BrickCVErrors : uint8_t
    {
        // Always add appropriate gerErrorMessage case
        UNIDENTIFIED = 0,
        NOT_SINGLE_CHANNEL_IMAGE = 1,
        COULD_NOT_ALLOCATE = 2,
        NULL_PTR = 3,
        MEMORY_LEAK_WARNING = 4, 
        IMAGE_PATH_EMPTY = 5,
        IMAGEs_NOT_EQUAL_SIZE = 6, 
        X_COORDINATE_MISMATCH = 7,
        FUNCTION_CALLED_TOO_SOON = 8, 
        EUCLIDIAN_DISTANCE_MISMATCH = 9
    };

    static const constexpr char* getErrorMessage(const BrickCVErrors& error)
    {
        switch (error)
        {
        case UNIDENTIFIED:
            return "[ERROR] \t [unspecified issue occured] \n";
            break;
        case NOT_SINGLE_CHANNEL_IMAGE:
            return "[ERROR] \t [This function requires the cv::Mat to only have one channel] \n";
            break;
        case COULD_NOT_ALLOCATE:
            return "[ERROR] \t [Memory for object wasn't allocated in operation] \n";
            break;
        case NULL_PTR:
            return "[WARNING] \t [Nullptr check prevented crash] \n";
            break;
        case MEMORY_LEAK_WARNING:
            return "[IMPORTANT ERROR] \t [There was an issue with logic effecting memory de-allocation] \n";
            break;
        case IMAGE_PATH_EMPTY:
            return "[IMPORTANT ERROR] \t [An image path was evaluated to an empty string] \n";
            break;
        case IMAGEs_NOT_EQUAL_SIZE:
            return "[IMPORTANT ERROR] \t [Function requires images of equal size] \n";
            break;
        case X_COORDINATE_MISMATCH:
            return "[CRITICAL ERROR] \t [Expected x coordinated diviated too strongly from provided] \n";
            break;
        case FUNCTION_CALLED_TOO_SOON:
            return "[CRITICAL ERROR] \t [This function failed a basic check it needs to pass in order to return a valid result, likely due to being called too soon. Please breakpoint and check] \n";
            break;
        case EUCLIDIAN_DISTANCE_MISMATCH:
            return "[WARNING] \t [HTML shades don't match expected colour] \n \t \t Suggestion: Breakpoint & check number of detected colours and current shade.\n";
            break;
        default:
            return "------------------------------------------------------------------- \n [UNKNOWN ERROR] \n \t I can't belive this. It done goofed!! How could this happen?? \n The anguish! \n Anyway, something went really wrong if you see this. Better check what happened... \n ------------------------------------------------------------------- \n";
        }
    }
}