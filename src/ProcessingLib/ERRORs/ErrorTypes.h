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
        COORDINATE_MISMATCH = 7,
        FUNCTION_CALLED_TOO_SOON = 8,
        EUCLIDIAN_DISTANCE_MISMATCH = 9,
        WRONG_COLOUR_CHANNEL = 10,
        COMPUT_MORE_EFFICENT_OPT_AVAILABLE = 11,
        COLOUR_SPACE_MISMATCH = 12,
        IMAGE_NOT_3_CHANNEL = 13
    };

    static const constexpr char* getErrorMessage(const BrickCVErrors& error)
    {
        // String format should be: "[Error level] \t [Error description] \n Optional debug suggestion \n \t \t"
        
        // [WARNING]          something that does not break the program but may mess with the result
        // [ERROR]            for simple program breaking errors
        // [IMPORTANT ERROR]  for memory effecting errors, where the effect is possibly invisible
        // [CRITICAL ERROR]   for errors in the program that will NOT break the program but lead to the wrong outcome

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
            return "[WARNING] \t [Nullptr check prevented crash] \n Suggestion: if still returned valid result this can possibly be ignored";
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
        case COORDINATE_MISMATCH:
            return "[CRITICAL ERROR] \t [Expected coordinated diviated too strongly from provided] \n";
            break;
        case FUNCTION_CALLED_TOO_SOON:
            return "[CRITICAL ERROR] \t [This function failed a basic check it needs to pass in order to return a valid result, likely due to being called too soon. Please breakpoint and check] \n";
            break;
        case EUCLIDIAN_DISTANCE_MISMATCH:
            return "[WARNING] \t [HTML shades don't match expected colour] \n \t \t Suggestion: Breakpoint & check number of detected colours and current shade.\n";
            break;
        case WRONG_COLOUR_CHANNEL:
            return "[ERROR] \t [Function call does not accomodate selected colour channel] \n";
            break;
        case COMPUT_MORE_EFFICENT_OPT_AVAILABLE:
            return "[WARNING] \t [This fucntion call is expensive] \n";
            break;
        case COLOUR_SPACE_MISMATCH:
            return "[ERROR] \t [This fucntion expects a different colour space] \n";
            break;
        case IMAGE_NOT_3_CHANNEL:
            return "[ERROR] \t [This fucntion expects a 3 channel image] \n";
            break;
        default:
            return "------------------------------------------------------------------- \n [UNKNOWN ERROR] \n \t \t I can't belive this. It done goofed!! How could this happen?? \n \t \t The anguish! \n \t \t Anyway, something went really wrong if you see this. Better check what happened... \n ------------------------------------------------------------------- \n";
        }
    }
}