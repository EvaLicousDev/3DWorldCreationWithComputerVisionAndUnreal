#pragma once

#include <string>
#include <fstream>
#include <filesystem>
#include <memory>
#include <iostream>

namespace legoCVTests
{
    struct CVAppOutputTester
    {
        static void testCSVOutput(const char* zDataFileName)
        {
            int linecount = 0;
            int tokenZCount = 0;
            std::ifstream in(zDataFileName);

            if (in.is_open())
            {
                std::string string{};
                std::string valueBuffer{};
                while (std::getline(in, string))
                {
                    std::stringstream substream{ string };
                    while (std::getline(substream, valueBuffer, ','))
                    {
                        std::cout << "--- val " << (tokenZCount + 1) << ": ";
                        // values have to be read as int, not uint 
                        int zValue = (int)(valueBuffer[0]);
                        std::cout << zValue << std::endl;
                        valueBuffer = std::string("");

                        tokenZCount++;
                    }

                    linecount++;
                }
                in.close();
            }

            std::cout << "CSV Reading Result: " << std::endl;
            std::cout << "------> Values Read: " << tokenZCount << std::endl;
            std::cout << "------> Lines Read:  " << linecount << std::endl;
        }
    };
}