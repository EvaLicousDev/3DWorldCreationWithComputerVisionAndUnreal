#pragma once
#include "TrainingInstanceDataStructs.h"
#include "../BrickCVEnums/BrickColourEnum.h"
#include "../BrickCVEnums/ChannelType.h"

#include <vector>
#include <memory>

//Open CV includes 
#include <opencv2/imgproc.hpp>
#include <opencv2/core/core.hpp>
namespace brickColourSVM
{
    class SVMClassifier
    {
        /* This class was written using the tutorial at https://docs.opencv.org/4.x/d1/d73/tutorial_introduction_to_svm.html */

        SVMClassifier(std::vector<UnprocessedTrainingDataInstance> trainingDataPaths)
        {
            // Set up training data
            int labels[4] = { 1, -1, -1, -1 };
            float trainingData[4][2] = { {501, 10}, {255, 10}, {501, 255}, {10, 501} };
            cv::Mat trainingDataMat(4, 2, CV_32F, trainingData);
            cv::Mat labelsMat(4, 1, CV_32SC1, labels);
        }
    };
}