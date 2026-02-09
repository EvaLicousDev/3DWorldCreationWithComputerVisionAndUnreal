#pragma once 

//standard library includes
#include <vector>
#include <memory>

//Open CV includes 
#include <opencv2/core/core.hpp>

namespace PreProcessor
{
	using imagePtr = std::shared_ptr<cv::Mat>;

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
		 *
		 */

	public:
		ImageProcessor(const cv::Mat& loadedImage)
			: in_image(loadedImage)
		{
		}

		//CONTINUE PAGE 78 for live thresholding

		void debugInfo(cv::Mat& image); 

		void display(int widthPixels, int heightPixels);
		void display(cv::Mat& image, int pixels = 500);

		cv::Mat naiveRgbColourSpaceReduction(cv::Mat& image, int divideBy = 16);
		cv::Mat naiveRgbColourSpaceReduction2(cv::Mat& image, int divideBy = 16);
		cv::Mat bitwiseRgbColourSpaceReduction(cv::Mat& image, int divideBy = 16);
		cv::Mat rgbColourSpaceReductionWithIt(cv::Mat& image, int divideBy = 16);

		cv::Mat sharpen2Dedges(cv::Mat& image); 

		cv::Mat& getMainImage() { return in_image; }
	private:
		cv::Mat in_image; 
	};
}
