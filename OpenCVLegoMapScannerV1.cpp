// OpenCVLegoMapScannerV1.cpp : Defines the entry point for the application.

//Project includes
#include "OpenCVLegoMapScannerV1.h"
#include "src/ProcessingLib/ImageProcessor.h"

//std lib includes
#include <memory>

using namespace std;
using namespace cv;
using namespace PreProcessor;

RNG rng(12345);

int main()
{
    //SUGGESTED ALG
    /*
     *Pre alg: 
     * 1. Get 5-10 example images of all colours
     * 2. Calculate histograms of isolated colour bricks
     *
     *Main:
     * 1. Find square and apply transform
     * 2. Use B/W bounderies to find threshold area of colours
     *      a. create areas of interest through finding widest bounding box of 4 sided shape
     * 3. Create mask to just use square
     *      a. reduce BGR colour space to 32-64 colours
     *      b. blur image heavily with k-means based on pixle transform and resize
     *      c. use red channel to isolate high green values and create a mask for the background
     *          i. create threshold mask by setting all pixles to either 0 or 255 if value < 25
     *      d. transform image to LAB
     *      e. use histogram back projection to threshold all colours
     *          i. for every colour's bounding box 
     *              1. use A & B channels to calculate likelyhood mask
     *              2. combine likelyhood masks for all colours for both channels
     *      f. Filter through bounding boxes of appropriate size > than min num pixls
     *          i. apply colour lable 1-8
     *      g. return coordinates & shapes of results
     */

    std::cout << "Starting Preprocessing" << std::endl;
    //const char* filepath = "C:/Users/evali/Pictures/Lego_TestPlate_GoodLighting2.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/Lego_TestPlate_GoodLighting2_testrow.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/WebcamLenovo_simple.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/Webcam_Envy360_complex.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/WebcamLenovo_glare_skew.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/Phone_Rosia_glare.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/Phone_Rosia_no_glare.jpg";

    //const char* filepath = "C:/Users/evali/Pictures/HDR.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/HDR_soft.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/HDR_vibrant.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/HDR_distance_zoom.jpg";

    const char* filepath = "C:/Users/evali/Pictures/HDR_night.jpg";
    //const char* filepath = "C:/Users/evali/Pictures/HDR_night2.jpg";
    

    cout << "----------------------------End--------------------------------" << endl;
    return 0;
}
