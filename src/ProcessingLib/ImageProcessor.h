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

namespace PreProcessor
{
    using imagePtr = std::shared_ptr<cv::Mat>;

    static const constexpr int sc_pixelsInHeightMap  = 2000; 
    static const constexpr int sc_coloursToIdentify  = 8;
    static const constexpr int sc_greenBlueLowerAxis = -126;
    static const constexpr int sc_greenBlueUpperAxis = 0;
    static const constexpr int sc_greenBlueLowerBY   = -128;
    static const constexpr int sc_greenBlueUpperBY   = 128;

    class ImageProcessor
    {
        /*
         * This class was created to encapsulate the preprocessing functions needed for detection of lego bricks in the images
         * provided by the camera.
         *
         * Goal:
         * Find the locations of different colour lego pieces in the image.
         *
         * Theory:
         * The RGB space provides 256 x 256 x 256 colours over three channels (approximately 16 mil).
         * Therefor we want to reduce the number of colours processed down to the minimum needed. For this we
         * divide the RGB space into "X" components where "X" stands for the number of configurable colours
         * for the Lego map we are trying to analyse.
         *
         * Approach:
         * We first apply some processing to identify the position of the boards from the lego bricks.
         * Then we isolate a mask for an area of interest over the "example plate" to apply some thresholding for colour values. 
         * This way we can dynamically set the colour ranges, allowing for slightly more resiliance to bad lighting. 
         * Note that colours are chosen in a way that allows for high contrast during processing. The AOI mask for example uses 
         * a transition from white to black to identify the AOI. 
         */

    public:
        ImageProcessor(const cv::Mat& loadedImage)
            : imageOfLego(std::make_shared<cv::Mat>(loadedImage))
        {
            if (!imageOfLego->empty())
            {
             //   visualiserInstance = std::make_unique<ColourSpaceVisualiser>(in_image); 
            }
        }

        ~ImageProcessor()
        {
            visualiserInstance.release();
        }

        std::weak_ptr<cv::Rect> getPlateRectangle() { return m_biggestRect; }
        std::vector<cv::Point2f> useContoursToFindCorners(cv::Mat original, cv::Mat& imageToProcess, bool showBlackMask = false, bool showAllRect = false);
        cv::Rect findChildCorners(int largestVoliumIndex, const std::vector<cv::Rect>& boxes, std::vector<cv::Vec4i> hierarchy, std::vector<std::vector<cv::Point>> contours, cv::Mat& preProcessedGrey, bool showResult);
        std::vector<cv::Rect> findBrickLocations(cv::Mat& brickArea, bool showResult = false); 

        cv::Point2f findTL(std::vector<std::vector<cv::Point>> contours, cv::Point middle);
        cv::Point2f findTR(std::vector<std::vector<cv::Point>> contours, cv::Point middle);
        cv::Point2f findBL(std::vector<std::vector<cv::Point>> contours, cv::Point middle);
        cv::Point2f findBR(std::vector<std::vector<cv::Point>> contours, cv::Point middle);

        cv::Rect findLargestVoliumSquareContour(std::vector<std::vector<cv::Point>>& boxes);
        cv::Rect findRectWithLongestSide(const std::vector<std::vector<cv::Point>>& contours, cv::Rect& topleftGreenCorner);
        cv::Rect findRectWithLargestVolium(const std::vector<cv::Rect>& boxes);
        cv::Rect findLegoWithThresholdingMask(cv::Mat imageToProcess, int lowerboundGreen, bool showGreenMask = false, bool showAllRect = false);
        cv::Rect findSquareIsh(const std::vector<cv::Point>& box);
        cv::Rect findLargestVoliumChild(std::vector<cv::Vec4i> hierarchy, std::vector<std::vector<cv::Point>> contours, cv::Mat& roi, bool showResult);
        cv::Rect getPlateWithGreenChannel(cv::Mat unprocessedROI, bool showResult); 
        void setXCoordinatesForWhiteBricks(std::vector<cv::Vec4i> hierarchy, std::vector<std::vector<cv::Point>> contours, cv::Mat& roi, bool showResult);

        cv::Mat applySobel(cv::Mat& blurredBGR, int k = 3);
       // cv::Mat customSobelEdges(cv::Mat& input1, cv::Mat& input2, cv::Mat& input3);
        cv::Mat applyCannyToBGR(cv::Mat& blurredBGR);
        cv::Mat applyCannyTo1D(cv::Mat& blurredGrey, int threshold);
        void    setContouringThresholds(cv::Mat& blurredGreyscale);
        void    getContourData(cv::Mat& allEdgesAdded, bool drawContours);

        cv::Mat reseizeImage(cv::Mat& image); 
        cv::Mat createThresholdMask(cv::Mat& greyImage);
        cv::Mat backprojectHistogram(cv::Mat& inputImage, cv::Mat& regionOfInterest, int threshold); 
        
        //image processing functions using pixle wise operations
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

        void    debugInfo(cv::Mat& image);
        cv::Mat getMSERMask(cv::Mat unblurredImage, bool showAreas = false);
        cv::Mat createMapWithContoursAndLABchannel(const cv::Mat& green, const cv::Mat& mser);

        cv::Mat                                               getDrawnContours() { return drawenContours; }
        std::shared_ptr<cv::Mat>                              getLegoPXMask()    { return legoPXMask;  }
        std::shared_ptr<std::vector<cv::Rect>>                getRectangles()    { return m_boundRect; }
        std::weak_ptr<std::vector<std::vector<cv::Point>>>    getContourPoints() { return m_contours;  }

        void setImageOfLego(cv::Mat& image); 
        void setImageOfBricks(cv::Mat& image);
        void setWhiteBrick(cv::Mat& image);
        std::shared_ptr<cv::Mat> getBrickSample() { return imageOfBricks; }

        void addToHeightMap(const cv::Mat& info); 
        std::weak_ptr<cv::Mat> getHeightMap() { return heightMap;  }
    private:
        bool isWithinTollerance(cv::Rect& output); 

       // cv::Mat bestEdges(cv::Mat& lumEdges, cv::Mat& axEdges, cv::Mat& byEdges);
        int leftWhiteMarkerTL_X = -1; 
        int rightWhiteMarkerTR_X = -1; 

        std::shared_ptr<cv::Mat> imageOfLego = nullptr; 
        std::shared_ptr<cv::Mat> imageOfBricks = nullptr;
        std::shared_ptr<cv::Mat> whiteBrick = nullptr;
        std::shared_ptr<cv::Mat> heightMap = nullptr;
        std::unique_ptr<ColourSpaceVisualiser> visualiserInstance = nullptr; 

        std::shared_ptr<cv::Mat> legoPXMask = nullptr;
        std::shared_ptr<cv::Mat> mserMask = nullptr;

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
        //cv::Mat createColourLocationImageBW(int maxDistance, const cv::Vec3b& tragetColour);
    };
}
