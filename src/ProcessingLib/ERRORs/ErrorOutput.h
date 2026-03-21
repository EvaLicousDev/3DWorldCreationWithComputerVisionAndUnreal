#pragma once
#include <iostream>
#include <fstream>
#include "windows.h"
#include "ErrorTypes.h"

namespace Errors
{
    // https://www.geeksforgeeks.org/cpp/how-to-print-colored-text-in-c/
    static const constexpr char* sc_errorOutputFilePath = "src/ProcessingLib/ERRORs/ErrorLogs/ErrorOutput.txt";

    class ErrorOutput
    {
    public:
        // This template class takes as many arguments in form of a string as passed in and prints them to the log and console
        // Note: This class should NOT receive string input from input streams!

        template<typename ...Args>
        ErrorOutput(BrickCVErrors errorType, Args&&... args)
        {
            //Print to console 
            std::cout.flush();
            system("Color 0D"); //temporarely make consol colour purple
            std::cout << Errors::getErrorMessage(errorType) << "\n"; 
            std::cout << "\t \t \t";
            addArgumentsToLine(std::cout, std::forward<Args>(args)...);

            //Print to log file 
            std::ofstream appendfile(sc_errorOutputFilePath, std::ios_base::app);
            if (appendfile.is_open())
            {
                appendfile << Errors::getErrorMessage(errorType) << "\n";
                appendfile << "\t \t \t"; 
                addArgumentsToLine(std::forward<std::ofstream>(appendfile), std::forward<Args>(args)...);
            }
            system("Color 0F");
        }

    private:
        template<typename FILE, typename ...Args>
        void addArgumentsToLine(FILE&& outStream, Args&&... args)
        {
            ((outStream << args << " "), ...) << "\n" << std::endl;
        }
    };
}
