#pragma once
#include "TrainingInstanceDataStructs.h"
#include "../BrickCVEnums/BrickColourEnum.h"
#include "../BrickCVEnums/ChannelType.h"

#include <vector>
#include <memory>

//Open CV includes 
#include <opencv2/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/ml.hpp>

using namespace BrickCV; 
namespace brickColourSVM
{
    /* This class was written using the tutorial at https://docs.opencv.org/4.x/d1/d73/tutorial_introduction_to_svm.html & https://docs.opencv.org/4.x/d0/dcc/tutorial_non_linear_svms.html */

    class SVMClassifier
    {
        // Given the system operates in varying light conditions and the single channel histogram backprojection experiment results, 
        // we can can observe that the data is generally not linearly seperable. 
        // Therefor a single instance of this class will create a 1 V ALL support vector machienes and noon linear kernals.
        // A collections of SVMClassifiers for all colours should then decide on the outcome

        const  int MAX_EXAMPLES = 10; //number of images per folder
        const BrickColour coloursToTrain[10] = {BrickColour::PURPLE, BrickColour::DARK_BLUE, BrickColour::LIGHT_BLUE, BrickColour::BROWN, BrickColour::RED, BrickColour::ORANGE, BrickColour::YELLOW, BrickColour::LIGHT_GREEN, BrickColour::DARK_GREEN, BrickColour::WHITE};

        SVMClassifier(BrickColour oneVSall)
        {
            theOne = oneVSall; 

            // --------------------Get all trainings data from folders-----------------------
            auto imagesPath = std::vector<cv::String>();
            auto labels = std::vector<int>(); 

            //populate folder paths
            for (auto colour : coloursToTrain)
            {
                auto folderpath = sc_trainingImageFolderPaths[colour]; 
                cv::String path(folderpath); 

                //Find every file in pattern following the "path" pattern
                cv::glob(path, imagesPath, false); 

                for (const auto imagePath : imagesPath)
                {
                    auto data = UnprocessedTrainingDataInstance(imagePath, colour); 
                    trainingData.emplace_back(data); 
                    int label = (colour == theOne ? 1 : -1); //where -1 is the label for any other colour than "theOne"
                    labels.emplace_back(label); 
                }
                imagesPath.clear(); 
            }
            // -------------------------------------------------------------------------------

            // ---------------------Format trainings data for OpenCV---------------------------

            //...
        }

    private:
        BrickColour theOne = BrickColour::BLACK; 
        std::vector<UnprocessedTrainingDataInstance> trainingData = std::vector<UnprocessedTrainingDataInstance>();
        std::vector<UnprocessedTrainingDataInstance> getTrainingImages(BrickCV::BrickColour colour); 
    };
}