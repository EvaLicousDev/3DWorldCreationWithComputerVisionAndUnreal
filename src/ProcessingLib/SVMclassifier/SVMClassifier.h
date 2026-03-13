#pragma once
#include "TrainingInstanceDataStructs.h"
#include "../BrickCVEnums/BrickColourEnum.h"
#include "../BrickCVEnums/ChannelType.h"

#include <vector>
#include <memory>

//Open CV includes 
#include <opencv2/imgproc.hpp>
#include <opencv2/core/core.hpp>
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
            auto images = std::vector<cv::String>();
            auto labels = std::vector<int>(); 
            //populate folder paths
            for (auto colour : coloursToTrain)
            {
                auto folderpath = sc_trainingImageFolderPaths[colour]; 
                cv::String path(folderpath); 

                //Find every file in pattern following the "path" pattern
                cv::glob(path, images, false); 

                for (auto imagePath : images)
                {
                    auto data = UnprocessedTrainingDataInstance(imagePath, colour); 
                    trainingData.emplace_back(data); 
                    int label = (colour == theOne ? 1 : -1); //where -1 is the label for any other colour than "theOne"
                    labels.emplace_back(label); 
                }
                images.clear(); 
            }
            // -------------------------------------------------------------------------------

            // ---------------------Format trainings data for OpenCV---------------------------
            int labels[4] = { 1, -1, -1, -1 };
            float trainingData[4][2] = { {501, 10}, {255, 10}, {501, 255}, {10, 501} };

            /*=
            // Train the SVM
            Ptr<SVM> svm = SVM::create();
            svm->setType(SVM::C_SVC);
            svm->setKernel(SVM::LINEAR);
            svm->setTermCriteria(TermCriteria(TermCriteria::MAX_ITER, 100, 1e-6));
            svm->train(trainingDataMat, ROW_SAMPLE, labelsMat);

            // Data for visual representation
            int width = 512, height = 512;
            Mat image = Mat::zeros(height, width, CV_8UC3);

            // Show the decision regions given by the SVM
            Vec3b green(0, 255, 0), blue(255, 0, 0);
            for (int i = 0; i < image.rows; i++)
            {
                for (int j = 0; j < image.cols; j++)
                {
                    Mat sampleMat = (Mat_<float>(1, 2) << j, i);
                    float response = svm->predict(sampleMat);

                    if (response == 1)
                        image.at<Vec3b>(i, j) = green;
                    else if (response == -1)
                        image.at<Vec3b>(i, j) = blue;
                }
            }

            // Show the training data
            int thickness = -1;
            circle(image, Point(501, 10), 5, Scalar(0, 0, 0), thickness);
            circle(image, Point(255, 10), 5, Scalar(255, 255, 255), thickness);
            circle(image, Point(501, 255), 5, Scalar(255, 255, 255), thickness);
            circle(image, Point(10, 501), 5, Scalar(255, 255, 255), thickness);

            // Show support vectors
            thickness = 2;
            Mat sv = svm->getUncompressedSupportVectors();

            for (int i = 0; i < sv.rows; i++)
            {
                const float* v = sv.ptr<float>(i);
                circle(image, Point((int)v[0], (int)v[1]), 6, Scalar(128, 128, 128), thickness);
            }

            imwrite("result.png", image);        // save the image

            imshow("SVM Simple Example", image); // show it to the user
            waitKey();
            */
        }

    private:
        BrickColour theOne = BrickColour::BLACK; 
        std::vector<UnprocessedTrainingDataInstance> trainingData = std::vector<UnprocessedTrainingDataInstance>();
        std::vector<UnprocessedTrainingDataInstance> getTrainingImages(BrickCV::BrickColour colour); 
    };
}