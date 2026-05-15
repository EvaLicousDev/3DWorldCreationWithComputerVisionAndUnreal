#pragma once 

//standard library includes
#include <vector>
#include <memory>

//Open CV includes 
#include <opencv2/imgproc.hpp>
#include <opencv2/core/core.hpp>

//project includes
#include "ColourSpaceVisualiser.h"

#define NEXT_SIBLING 0
#define PREV_SIBLING 1
#define CHILD_CONTOUR 2
#define PARENT_CONTOUR 3
#define STUDS_PER_PLATE 32

static constexpr size_t channels = 3;
static constexpr size_t blueOrLuminance = 0;
static constexpr size_t greenOrAxis = 1;
static constexpr size_t redOrBlueYellow = 2;

static constexpr size_t approximateLegoKnobPixels = 16;

namespace ImageProcessing
{
    using imagePtr = std::shared_ptr<cv::Mat>;

    // previous scale was adjusted for implementation reasons to do on the unreal engine code side
    // static const constexpr int sc_pixelsInHeightMap  = 2017*2; //https://dev.epicgames.com/documentation/en-us/unreal-engine/landscape-technical-guide-in-unreal-engine
    static const constexpr int sc_pixelsInHeightMap = 400; 
    static const constexpr int sc_coloursToIdentify  = 8;
    static const constexpr int sc_greenBlueLowerAxis = -127;
    static const constexpr int sc_greenBlueUpperAxis = 0;
    static const constexpr int sc_greenBlueLowerBY   = -126;
    static const constexpr int sc_greenBlueUpperBY   = 127;

    class ImageProcessor
    {
        /*
         * This class was created to encapsulate the preprocessing functions needed for detection of lego bricks in the images
         * provided by the camera.
         * 
         * Colour spaces used to identify bricks are primarely LAB and LCH. Please also view BrickCVEnums/BrickColourEnum.h for more comments.
         * LAB was chosen due to the initial literature review and testing & LCH was added to improve detection further
         *
         * Approach:
         * We first apply some processing to identify the position of the boards from the lego bricks.
         * Then we isolate a mask for an area of interest over the "example plate" to apply some thresholding for colour values. 
         * This way we can dynamically set the colour ranges rather than tying them to set values, allowing for slightly more resiliance to bad lighting. 
         */

    public:
        ImageProcessor(const cv::Mat& loadedImage)
            : imageOfLego(std::make_shared<cv::Mat>(loadedImage))
        {
            if (!imageOfLego->empty())
            {
             //   in early development this was used to diplay and visually test the images loaded by this application
             //   visualiserInstance = std::make_unique<ColourSpaceVisualiser>(in_image); 
            }
        }

        ~ImageProcessor()
        {
            visualiserInstance.release();
        }

        cv::Mat boostValue(const cv::Mat& input, bool showResult);
        std::weak_ptr<cv::Rect>  getPlateRectangle() { return m_biggestRect; }
        std::vector<cv::Point2f> useContoursToFindCorners(cv::Mat original, cv::Mat& imageToProcess, bool showBlackMask = false, bool showAllRect = false);
        cv::Rect                 findChildCorners(int largestVoliumIndex, const std::vector<cv::Rect>& boxes, std::vector<cv::Vec4i> hierarchy, std::vector<std::vector<cv::Point>> contours, cv::Mat& preProcessedGrey, bool showResult = false);
        std::vector<cv::Rect>    findBrickLocations(cv::Mat& brickArea, bool showResult = false); 
        cv::Mat                  createHeightMapPNG(cv::Mat& heightMapData, const char* heightmapOutputPath); 
        void                     addToHeightMap(const cv::Mat& info);
        std::weak_ptr<cv::Mat>   getHeightMap() { return m_heightMap; }

        cv::Point2f findTL(std::vector<std::vector<cv::Point>> contours, cv::Point middle);
        cv::Point2f findTR(std::vector<std::vector<cv::Point>> contours, cv::Point middle);
        cv::Point2f findBL(std::vector<std::vector<cv::Point>> contours, cv::Point middle);
        cv::Point2f findBR(std::vector<std::vector<cv::Point>> contours, cv::Point middle);

        cv::Mat createRetinex(const cv::Mat& input, bool showResult = false);
        cv::Mat adaptiveShadowRemovalMask(const cv::Mat& input, const cv::Mat& retinex, double sensitivity = 1.0, bool showResult = false);
        cv::Mat removeShadows(const cv::Mat& input, cv::Mat& retinex, cv::Mat mask = cv::Mat(), bool showResult = false);

        // DEPRECATED
        cv::Rect findLargestVoliumSquareContour(std::vector<std::vector<cv::Point>>& boxes);
        cv::Rect findRectWithLongestSide(const std::vector<std::vector<cv::Point>>& contours, cv::Rect& topleftGreenCorner);
        cv::Rect findRectWithLargestVolium(const std::vector<cv::Rect>& boxes);
        cv::Rect findLegoWithThresholdingMask(cv::Mat imageToProcess, int lowerboundGreen, bool showGreenMask = false, bool showAllRect = false);
        cv::Rect findSquareIsh(const std::vector<cv::Point>& box);
        cv::Rect findLargestVoliumChild(std::vector<cv::Vec4i> hierarchy, std::vector<std::vector<cv::Point>> contours, cv::Mat& roi, bool showResult = false);
        cv::Rect getPlateWithGreenChannel(cv::Mat unprocessedROI, bool showResult = false); 
        void     setXCoordinatesForWhiteBricks(std::vector<cv::Vec4i> hierarchy, std::vector<std::vector<cv::Point>> contours, cv::Mat& roi, bool showResult = false);
        //---------------

        cv::Mat applySobel(cv::Mat& blurredBGR, int k = 3);
        cv::Mat applyCannyToBGR(cv::Mat& blurredBGR);
        cv::Mat applyCannyTo1D(cv::Mat& blurredGrey, int threshold);
        void    setContouringThresholds(cv::Mat& blurredGreyscale);
        void    getContourData(cv::Mat& allEdgesAdded, bool drawContours);

        cv::Mat reseizeImage(cv::Mat& image); 
        cv::Mat createThresholdMask(cv::Mat& greyImage);
        cv::Mat backprojectHistogram(cv::Mat& inputImage, cv::Mat& regionOfInterest, int threshold); 
        
        cv::Mat naiveRgbColourSpaceReduction(cv::Mat& image,   int divideBy = 16);
        cv::Mat naiveRgbColourSpaceReduction2(cv::Mat& image,  int divideBy = 16);
        cv::Mat bitwiseRgbColourSpaceReduction(cv::Mat& image, int divideBy = 16);
        cv::Mat rgbColourSpaceReductionWithIt(cv::Mat& image,  int divideBy = 16);

        static cv::Mat thresholdColourOnChannel(cv::Mat channel, int lowerBound, int upperBound, const char* frameName, bool showImage = false);
        cv::Mat        backprojectHistogramHSV(cv::Mat& inputImage, cv::Mat& regionOfInterest, int threshold); 

        cv::Mat sharpen2Dedges(cv::Mat& image); 
        int     getDistanceToTargetColour(const cv::Vec3b& colourIn, const cv::Vec3b& tragetColour) const; 

        //debug functions for development
        cv::Mat& getMainImage() { return *imageOfLego; }
        void     debugInfo(cv::Mat& image);
        cv::Mat  getMSERMask(cv::Mat unblurredImage, bool showAreas = false);
        cv::Mat  createMapWithContoursAndLABchannel(const cv::Mat& green, const cv::Mat& mser);

        cv::Mat                                               getDrawnContours() { return drawenContours; }
        std::shared_ptr<cv::Mat>                              getLegoPXMask()    { return legoPXMask;  }
        std::shared_ptr<std::vector<cv::Rect>>                getRectangles()    { return m_boundRect; }
        std::weak_ptr<std::vector<std::vector<cv::Point>>>    getContourPoints() { return m_contours;  }

        void setImageOfLego(cv::Mat& image); 
        void setImageOfBricks(cv::Mat& image);
        void setWhiteBrick(cv::Mat& image);
        std::shared_ptr<cv::Mat> getBrickSample() { return imageOfBricks; }
    private:
        bool isWithinTollerance(cv::Rect& output); 

        int leftWhiteMarkerTL_X  = -1; 
        int rightWhiteMarkerTR_X = -1; 

        std::shared_ptr<cv::Mat> imageOfLego   = nullptr; 
        std::shared_ptr<cv::Mat> imageOfBricks = nullptr;
        std::shared_ptr<cv::Mat> whiteBrick    = nullptr;
        std::shared_ptr<cv::Mat> m_heightMap   = nullptr;
        std::unique_ptr<ColourSpaceVisualiser> visualiserInstance = nullptr; 

        std::shared_ptr<cv::Mat> legoPXMask = nullptr;
        std::shared_ptr<cv::Mat> mserMask   = nullptr;

        std::shared_ptr<cv::Mat> image_L = nullptr;
        std::shared_ptr<cv::Mat> image_A = nullptr;
        std::shared_ptr<cv::Mat> image_B = nullptr;
        std::shared_ptr<cv::Mat> image_S = nullptr;

        std::shared_ptr<std::vector<std::vector<cv::Point>>> m_contours = nullptr;
        std::shared_ptr<std::vector<cv::Rect>>               m_boundRect     = nullptr;

        cv::Mat drawenContours;
        std::shared_ptr<cv::Rect> m_biggestRect = nullptr; 

        double controurThreshMax    = 0;
        double controurThreshMin    = 0;
        double controurThreshMiddle = 0;
    };
}
