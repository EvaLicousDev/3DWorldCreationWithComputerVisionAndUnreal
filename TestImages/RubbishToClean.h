/*
// Saturation for initial detection of plate
    // GREEN channel + LAB  for colour identification
    // Take image as is and get the Value channel or greyscale

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

cv::Mat notBlured = cv::imread(filepath, cv::IMREAD_COLOR_BGR);
auto numVerticalPixl = notBlured.rows;
auto desiredSizeFactorVertical = numVerticalPixl / 500;
auto desiredSize = numVerticalPixl / desiredSizeFactorVertical;
float reseizeFactor = static_cast<float>(desiredSize) / static_cast<float>(numVerticalPixl);

cv::Mat small;
cv::Mat image;
cv::Mat greyScale, red;
cv::Mat lumosity, axis, by;
cv::Mat sobelx, sobely, gradient;
cv::Mat gradient_abs_l;
cv::Mat gradient_abs_a;
cv::Mat gradient_abs_b;
cv::Mat gradient_abs_grey, gradient_abs_green;;

cv::resize(notBlured, small, cv::Size(), reseizeFactor, reseizeFactor, cv::INTER_LINEAR);
cv::cvtColor(small, greyScale, cv::COLOR_BGR2GRAY);


std::unique_ptr<ImageProcessor> preProcessor = std::make_unique<ImageProcessor>(small);
lumosity = preProcessor->getLuminance();
axis = preProcessor->getAxis();
by = preProcessor->getBY();
red = preProcessor->getRed();

cv::normalize(lumosity, lumosity, 0, 100, cv::NORM_MINMAX);
cv::normalize(axis, axis, 0, 100, cv::NORM_MINMAX);
cv::normalize(by, by, 0, 100, cv::NORM_MINMAX);

preProcessor->display(small, "Original Resized");
//preProcessor->display(greyScale, "Greyscale");

//cv::Mat greyThresh;
//cv::GaussianBlur(greyScale, greyScale, cv::Size(), 1.4);
//cv::threshold(greyScale, greyThresh, 150, 255, ADAPTIVE_THRESH_MEAN_C);
//preProcessor->display(greyThresh, "greyscale thresholds");

//cv::Mat grey2 = imread(filepath, IMREAD_REDUCED_GRAYSCALE_2);
cv::Mat grey3 = imread(filepath, IMREAD_REDUCED_GRAYSCALE_4);
//cv::Mat grey4 = imread(filepath, IMREAD_REDUCED_GRAYSCALE_8);
//preProcessor->display(grey2, "Reduced GS 2");
//preProcessor->display(grey3, "Reduced GS 4");
//preProcessor->display(grey4, "Reduced GS 8");

//cv::GaussianBlur(grey2, grey2, cv::Size(), 1.4);
cv::GaussianBlur(grey3, grey3, cv::Size(), 1.4);
preProcessor->naiveRgbColourSpaceReduction2(grey3, 16);

//cv::GaussianBlur(grey4, grey4, cv::Size(), 1.4);
//preProcessor->display(grey2, "Reduced GS 2");
preProcessor->display(grey3, "Reduced GS 4");
//preProcessor->display(grey4, "Reduced GS 8");

//cv::threshold(grey2, grey2, 150, 255, ADAPTIVE_THRESH_MEAN_C);
//preProcessor->display(grey2, "greyscale thresholds");
//cv::threshold(grey3, grey3, 150, 255, ADAPTIVE_THRESH_MEAN_C);
//preProcessor->display(grey3, "greyscale thresholds");
//cv::threshold(grey4, grey4, 150, 255, ADAPTIVE_THRESH_MEAN_C);
//preProcessor->display(grey4, "greyscale thresholds");

//    cv::Mat gsEdges; 
//    //cv::Canny(grey2, gsEdges, 30, 120);
////    preProcessor->display(gsEdges, "Reduced GS 2");
//    cv::Canny(grey3, gsEdges, 30, 120);
//    preProcessor->display(gsEdges, "Reduced GS 3");
    //cv::Canny(grey4, gsEdges, 30, 120);
  //  preProcessor->display(gsEdges, "Reduced GS 4");

    //preProcessor->display(lumosity, "Luminance");
    //preProcessor->displayRGBChannels();
    //preProcessor->displayHSVChannels();
    //preProcessor->displayLABChannels();
    //preProcessor->displayLUVChannels();

cv::Mat processed = preProcessor->naiveRgbColourSpaceReduction(preProcessor->getMainImage(), 64);
cv::Mat blurred;
cv::bilateralFilter(processed, blurred, 30, 60, 15);
preProcessor->updateHSVChannelsWithProcessed(blurred);
lumosity = preProcessor->getLuminance();
axis = preProcessor->getAxis();
by = preProcessor->getBY();
red = preProcessor->getRed();

//cv::normalize(lumosity, lumosity, 0, 255, cv::NORM_MINMAX);
//cv::normalize(axis, axis, 0, 255, cv::NORM_MINMAX);
//cv::normalize(by, by, 0, 255, cv::NORM_MINMAX);
cv::normalize(red, red, 0, 255, cv::NORM_MINMAX);

preProcessor->display(red, "RED");

cv::Mat redHist;
float hranges[2] = { 0,255 };
const float* ranges[1];
ranges[0] = hranges;
int sizeHist[1] = { 256 };
int channels[1] = { 0 };
cv::Mat images[1] = { red };
cv::calcHist(images, 1, channels, cv::Mat(), redHist, 1, sizeHist, ranges, true, false);

double minVal;
double maxVal;
cv::minMaxLoc(redHist, &minVal, &maxVal, 0, 0);

auto histImage = cv::Mat(redHist.rows, redHist.rows, CV_8UC1, Scalar(255));
int highest = static_cast<int>(90 * redHist.rows);

for (int h = 0; h < redHist.rows; h++)
{
    float binVal = redHist.at<float>(h);
    if (binVal > 0)
    {
        int intensity = static_cast<int>(binVal * highest / maxVal);
        cv::line(histImage, cv::Point(h, redHist.rows), cv::Point(h, redHist.rows - intensity), cv::Scalar(0), 1);
    }
}

preProcessor->display(histImage, "Red Hist Attempt");

auto claheAlg = cv::createCLAHE(50, Size(48, 48));
cv::Mat clahedImage;
cv::Mat blurredGreyscale;
cv::cvtColor(blurred, blurredGreyscale, COLOR_BGR2GRAY);
claheAlg->apply(blurredGreyscale, clahedImage);



// preProcessor->display(blurred, "Colour Reduction");
 //preProcessor->display(clahedImage, "CLAHED");

 //preProcessor->displayRGBChannels();
 //preProcessor->displayHSVChannels();
 //preProcessor->displayLABChannels();
 //preProcessor->displayLUVChannels();

 //// Apply Sobel operator
 //cv::Sobel(grey3, sobelx, CV_64F, 1, 0, 3);
 //cv::Sobel(grey3, sobely, CV_64F, 0, 1, 3);
 //// Compute gradient magnitude
 //cv::magnitude(sobelx, sobely, gradient);
 //// Convert to 8-bit image
 //cv::convertScaleAbs(gradient, gradient_abs_green);
 //// Display result
 //preProcessor->display(gradient_abs_green, "Clahed Sobel edge detection");

// vector<vector<Point> > contours2;
 //cv::findContours(gradient_abs_green, contours, RETR_TREE, CHAIN_APPROX_SIMPLE);

 // Apply Sobel operator
cv::Sobel(blurredGreyscale, sobelx, CV_64F, 1, 0, 3);
cv::Sobel(blurredGreyscale, sobely, CV_64F, 0, 1, 3);
// Compute gradient magnitude
cv::magnitude(sobelx, sobely, gradient);
// Convert to 8-bit image
cv::convertScaleAbs(gradient, gradient_abs_grey);
// Display result
cv::imshow("Grey - Sobel Edge Detection", gradient_abs_grey);
//cv::waitKey(0);

// Apply Sobel operator
cv::Sobel(lumosity, sobelx, CV_64F, 1, 0, 3);
cv::Sobel(lumosity, sobely, CV_64F, 0, 1, 3);
// Compute gradient magnitude
cv::magnitude(sobelx, sobely, gradient);
// Convert to 8-bit image
cv::convertScaleAbs(gradient, gradient_abs_l);
// Display result
cv::imshow("Lum - Sobel Edge Detection", gradient_abs_l);
cv::waitKey(0);

// Apply Sobel operator
cv::Sobel(axis, sobelx, CV_64F, 1, 0, 3);
cv::Sobel(axis, sobely, CV_64F, 0, 1, 3);
// Compute gradient magnitude
cv::magnitude(sobelx, sobely, gradient);
// Convert to 8-bit image
cv::convertScaleAbs(gradient, gradient_abs_a);
// Display result
// cv::imshow("Axi - Sobel Edge Detection", gradient_abs_a);
 //cv::waitKey(0);

 // Apply Sobel operator
cv::Sobel(by, sobelx, CV_64F, 1, 0, 3);
cv::Sobel(by, sobely, CV_64F, 0, 1, 3);
// Compute gradient magnitude
cv::magnitude(sobelx, sobely, gradient);
// Convert to 8-bit image
cv::convertScaleAbs(gradient, gradient_abs_b);
// Display result
//cv::imshow("BY - Sobel Edge Detection", gradient_abs_b);
//cv::waitKey(0);

cv::Mat allEdgesAdded = preProcessor->bestEdges(gradient_abs_l, gradient_abs_a, gradient_abs_b);
//cv::add(gradient_abs_green, allEdgesAdded, allEdgesAdded);
//cv::add(gradient_abs_green, gradient_abs_b, allEdgesAdded);
//cv::add(gradient_abs_b, allEdgesAdded, allEdgesAdded);
//cv::add(allEdgesAdded, gradient_abs_a, allEdgesAdded);
//cv::add(allEdgesAdded, gradient_abs_a, allEdgesAdded);
//cv::add(gradient_abs_grey, allEdgesAdded, allEdgesAdded);


cv::imshow("All Edges Added up", allEdgesAdded);
cv::waitKey(0);

vector<vector<Point> > contours;
//cv::threshold(gradient_abs_l, gradient_abs_l, 200, 255, THRESH_BINARY);
cv::findContours(allEdgesAdded, contours, RETR_TREE, CHAIN_APPROX_SIMPLE);
//vector<vector<Point> > contours_poly(contours.size());
//vector<Rect> boundRect(contours.size());
//vector<Point2f>centers(contours.size());
//vector<float>radius(contours.size());
//auto drawn = Mat(gradient_abs_l.cols, gradient_abs_l.rows, CV_8UC1);
//for (size_t i = 0; i < contours.size(); i++)
//{
//    approxPolyDP(contours[i], contours_poly[i], 3, true);
//    boundRect[i] = boundingRect(contours_poly[i]);
//    minEnclosingCircle(contours_poly[i], centers[i], radius[i]);
//}
//for (size_t i = 0; i < contours.size(); i++)
//{
//    Scalar color = Scalar(rng.uniform(0, 256), rng.uniform(0, 256), rng.uniform(0, 256));
//    drawContours(blurred, contours_poly, (int)i, color);
//    drawContours(drawn, contours_poly, (int)i, color);
//    rectangle(blurred, boundRect[i].tl(), boundRect[i].br(), color, 2);
//    //circle(small, centers[i], (int)radius[i], color, 2);
//}
//preProcessor->display(drawn, "Contours", false);
//preProcessor->display(blurred, "drawn contours with soble", false);

//// Apply Gaussian blur
//cv::Mat edges; 
//cv::Mat blurColour = cv::imread(filepath, IMREAD_GRAYSCALE); 
//cv::GaussianBlur(blurColour, blurColour, cv::Size(), 1.2);
//// Apply Canny Edge Detector
//cv::Canny(blurColour, edges, 50, 100);
//cv::resize(edges, edges, cv::Size(), 0.35, 0.35, cv::INTER_LINEAR);
//// Display result
//cv::imshow("Canny Edge Detection Greyscale", edges);

preProcessor->display(blurredGreyscale, "grescale for canny");

double max;
double min;
double middle;
normalize(blurredGreyscale, blurredGreyscale, 0, 255, NORM_MINMAX);
cv::minMaxIdx(blurredGreyscale, &min, &max, 0, 0);
middle = (min + max) / 2;
double maxThreshCan = middle * 1.5;
if (maxThreshCan > 255)
maxThreshCan = 255;
double minThreshCan = maxThreshCan * 0.9;

// Apply Gaussian blur
cv::Mat edges;
// cv::Mat blurColour;
 //cv::cvtColor(allEdgesAdded, blurColour, COLOR_BGR2GRAY);
 //cv::GaussianBlur(allEdgesAdded, blurColour, cv::Size(), 1.2);
 // Apply Canny Edge Detector
cv::Canny(blurredGreyscale, edges, minThreshCan, maxThreshCan);
//cv::resize(edges, edges, cv::Size(), 0.6, 0.6, cv::INTER_LINEAR);
// Display result
//cv::imshow("Canny Edge Detection Greyscale", edges);
preProcessor->display(edges, "Canny Edge detection before contour");

cv::findContours(allEdgesAdded, contours, RETR_TREE, CHAIN_APPROX_SIMPLE);
vector<vector<Point> > contours_poly(contours.size());
vector<Rect> boundRect(contours.size());
vector<Point2f>centers(contours.size());
vector<float>radius(contours.size());
auto drawn = Mat(gradient_abs_l.cols, gradient_abs_l.rows, CV_8UC1);
for (size_t i = 0; i < contours.size(); i++)
{
    approxPolyDP(contours[i], contours_poly[i], 4, true);
    boundRect[i] = boundingRect(contours_poly[i]);
    minEnclosingCircle(contours_poly[i], centers[i], radius[i]);
}

int numBiggest = 0;
int idBiggest = 0;
for (int i = 0; i < boundRect.size(); i++) {
    if (boundRect[i].width > numBiggest) {
        numBiggest = boundRect[i].width;
        idBiggest = i;
    }
}

double biggestWidth = boundRect[idBiggest].width;
double about1Square = biggestWidth / 32;
// about1Square = about1Square * 0.95; 

for (size_t i = 0; i < contours.size(); i++)
{
    if (boundRect[i].width >= about1Square && boundRect[i].height >= about1Square)
    {
        Scalar color = Scalar(rng.uniform(0, 256), rng.uniform(0, 256), rng.uniform(0, 256));
        //drawContours(blurred, contours_poly, (int)i, color);
        //drawContours(drawn, contours_poly, (int)i, color);
        rectangle(blurred, boundRect[i].tl(), boundRect[i].br(), color, 2);
        //circle(small, centers[i], (int)radius[i], color, 2);
    }
}

//Scalar color = Scalar(rng.uniform(0, 256), rng.uniform(0, 256), rng.uniform(0, 256));
//drawContours(blurred, contours_poly, idBiggest, color);
//drawContours(drawn, contours_poly, idBiggest, color);
//rectangle(blurred, boundRect[idBiggest].tl(), boundRect[idBiggest].br(), color, 2);

preProcessor->display(drawn, "Contours", false);
preProcessor->display(blurred, "drawn contours with custom edge detect", false);

//preProcessor->display(processed, "Filtered");
//preProcessor->updateHSVChannelsWithProcessed(processed);
//preProcessor->displayRGBChannels();
//preProcessor->displayHSVChannels();
//preProcessor->displayLABChannels();
//preProcessor->displayLUVChannels();


cout << "----------------------------End--------------------------------" << endl;
return 0;
}
*/