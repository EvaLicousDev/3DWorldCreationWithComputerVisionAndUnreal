// #include "HDRImageArrayMerger.h"
//
// #include <opencv2/imgcodecs.hpp>
// #include <opencv2/core/mat.hpp>
// #include "opencv2/photo.hpp"
//
// #include "../ERRORs/ErrorOutput.h"
//
// cv::Mat brickHDR::CustomHDRImageArrayMerger::mergeImages()
// {
//     std::vector<cv::Mat> images;
//     std::vector<float> times;
//     loadloadExposureSequence(m_imagesPath, images, times);
//
//     cv::Mat response;
//     cv::Ptr<cv::CalibrateDebevec> calibrate = cv::createCalibrateDebevec();
//     calibrate->process(images, response, times);
//
//     cv::Mat hdr;
//     cv::Ptr<cv::MergeDebevec> merge_debevec = cv::createMergeDebevec();
//     merge_debevec->process(images, hdr, times, response);
//
//     cv::Mat ldr;
//     cv::Ptr<cv::Tonemap> tonemap = cv::createTonemap(2.2f);
//     tonemap->process(hdr, ldr);
//
//     cv::Mat fusion;
//     cv::Ptr<cv::MergeMertens> merge_mertens = cv::createMergeMertens();
//     merge_mertens->process(images, fusion);
//
//     // cv::imwrite("fusion.png", fusion * 255);
//     // cv::imwrite("ldr.png", ldr * 255);
//     // imwrite("hdr.hdr", hdr);
//
//     return fusion; 
// }
//
// void brickHDR::CustomHDRImageArrayMerger::loadloadExposureSequence(const char* path, std::vector<cv::Mat>& images, std::vector<float>& times)
// {
//     if (std::strcmp("",path) !=0)
//     {
//         path = path + "/";
//         std::ifstream list_file((path + "list.txt").c_str());
//         std::string name;
//         float val;
//
//         while (list_file >> name >> val) {
//             //reads txt file in folder like
//             // [image name][ ][exposure as float]
//             // example: 
//             // someImage.jpeg 0.000333
//
//             cv::Mat img = cv::imread(path + name);
//             images.push_back(img);
//             times.push_back(1 / val);
//         }
//         list_file.close();
//     } 
//     else
//     {
//         Errors::ErrorOutput(Errors::BrickCVErrors::IMAGE_PATH_EMPTY, "The path given for the folder of HDR images is empty. Did the ref go out of scope?"); 
//     }
// }
//
