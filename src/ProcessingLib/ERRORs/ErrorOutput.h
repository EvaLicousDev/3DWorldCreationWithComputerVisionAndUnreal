#pragma once
#include <iostream>
#include <fstream>
#include "ErrorTypes.h"

namespace Errors
{
    class ErrorOutput
    {
    public:
        // This template class takes as many arguments in form of a string as passed in and prints them to the log and console
        // Note: This class should NOT receive string input from input streams!

        template<typename ...Args>
        ErrorOutput(BrickCVErrors errorType, Args&&... args)
        {
            //Print to console 
            std::cout << Errors::getErrorMessage(errorType) << " "; 
            addArgumentsToLine(std::cout, std::forward<Args>(args)...);

            //Print to log file 
            std::ofstream file(outputFilePath, std::ios_base::app);
            if (file.is_open())
            {
                file << Errors::getErrorMessage(errorType) << " ";
                addArgumentsToLine(std::forward<std::ofstream>(file), std::forward<Args>(args)...);
            }
        }

    private:

        template<typename FILE, typename ...Args>
        void addArgumentsToLine(FILE&& outStream, Args&&... args)
        {
            ((outStream << args << " "), ...) << std::endl; 
        }

        const char* outputFilePath = "ErrorLogs/ErrorOutput.txt";
    };
}
