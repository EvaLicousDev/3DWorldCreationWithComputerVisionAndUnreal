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
        FUNCTION_CALLED_TOO_SOON = 8
    };

    static const constexpr char* getErrorMessage(const BrickCVErrors& error)
    {
        switch (error)
        {
        case UNIDENTIFIED:
            return "[ERROR] [unspecified issue occured]";
            break;
        case NOT_SINGLE_CHANNEL_IMAGE:
            return "[ERROR] [This function requires the cv::Mat to only have one channel]";
            break;
        case COULD_NOT_ALLOCATE:
            return "[ERROR] [Memory for object wasn't allocated in operation]";
            break;
        case NULL_PTR:
            return "[WARNING] [Nullptr check prevented crash]";
            break;
        case MEMORY_LEAK_WARNING:
            return "[IMPORTANT ERROR] [There was an issue with logic effecting memory de-allocation]";
            break;
        case IMAGE_PATH_EMPTY:
            return "[IMPORTANT ERROR] [An image path was evaluated to an empty string]";
            break;
        case IMAGEs_NOT_EQUAL_SIZE:
            return "[IMPORTANT ERROR] [Function requires images of equal size]";
            break;
        case X_COORDINATE_MISMATCH:
            return "[CRITICAL ERROR] [Expected x coordinated diviated too strongly from provided]";
            break;
        case FUNCTION_CALLED_TOO_SOON:
            return "[CRITICAL ERROR] [This function failed a basic check it needs to pass in order to return a valid result, likely due to being called too soon. Please breakpoint and check.]";
            break;
        default:
            return "sth went really wrong if you see this.";
        }
    }
}