#include "ForegroundDetector.h"

using std::endl;
using std::cerr;

//-----------------------------------------------------------------------
//  class ImageDifference implementation
//-----------------------------------------------------------------------

ImageDifference::ImageDifference(void)
{
    // Update background model by default
    updateFlag = true;
}

ImageDifference::~ImageDifference(void)
{
}

void ImageDifference::process(Mat &img)
{
    try {
        frame = img.clone();

        // Initialize background model
        if (bgImage.empty()) {
            cvtColor(frame, bgImage, CV_BGR2GRAY);
            return;
        }

        // Convert to gray scale image
        cvtColor(frame, gray, CV_BGR2GRAY);

        // Subtract the current frame with background model to obtain
        // foreground mask.
        absdiff(bgImage, gray, fgMask);

        // Set pixels larger than 16 to 255, otherwise zero
        threshold(fgMask, fgMask, 16, 255, THRESH_BINARY);

        //------------------------------------------------------------
        //  Update background model
        //------------------------------------------------------------

        //  Background model is simply the previous frame
        if (updateFlag) {
            // keep the current frame as background model
            cvtColor(frame, bgImage, CV_BGR2GRAY);
        }

    } catch ( cv::Exception &e ) {
        const char *errMsg = e.what();
        cerr << "{ImageDifference}[ERROR] process(): " << errMsg << endl;
    }
}

void ImageDifference::draw()
{
    namedWindow("Foreground Mask", CV_WINDOW_AUTOSIZE);
    imshow("Foreground Mask", fgMask);
    waitKey(1);
}

//-----------------------------------------------------------------------
//  class ForegroundDetectorMOG implementation
//-----------------------------------------------------------------------

ForegroundDetectorMOG::ForegroundDetectorMOG(void)
{
    // Update background model by default
    updateFlag = true;
    bgSubtractor = createBackgroundSubtractorMOG();
}

ForegroundDetectorMOG::~ForegroundDetectorMOG(void)
{
}

Mat& ForegroundDetectorMOG::getBackgroundImage()
{
    bgSubtractor->getBackgroundImage(bgImage);
    return bgImage;
}

void ForegroundDetectorMOG::process(Mat &img)
{
    try {
        frame = img.clone();

        //------------------------------------------------------------
        //  Update background model
        //------------------------------------------------------------

        if (updateFlag) {
            bgSubtractor->apply(frame, fgMask, -1);
        } else {
            bgSubtractor->apply(frame, fgMask, 0);
        }
    } catch ( cv::Exception &e ) {
        const char *errMsg = e.what();
        cerr << "{ForegroundDetectorMOG}[ERROR] process(): " << errMsg << endl;
    }
}

void ForegroundDetectorMOG::draw()
{
    namedWindow("Foreground Mask", CV_WINDOW_AUTOSIZE);
    imshow("Foreground Mask", fgMask);
    waitKey(1);
}

//-----------------------------------------------------------------------
//  class ForegroundDetectorMOG2 implementation
//-----------------------------------------------------------------------

ForegroundDetectorMOG2::ForegroundDetectorMOG2(void)
{
    // Update background model by default
    updateFlag = true;
    bgSubtractor = createBackgroundSubtractorMOG2();
}

ForegroundDetectorMOG2::~ForegroundDetectorMOG2(void)
{
}

Mat& ForegroundDetectorMOG2::getBackgroundImage()
{
    bgSubtractor->getBackgroundImage(bgImage);
    return bgImage;
}

void ForegroundDetectorMOG2::process(Mat &img)
{
    try {
        frame = img.clone();

        //------------------------------------------------------------
        //  Update background model
        //------------------------------------------------------------

        if (updateFlag) {
            bgSubtractor->apply(frame, fgMask, -1);
        } else {
            bgSubtractor->apply(frame, fgMask, 0);
        }

        // Ignore shadow pixels
        inRange(fgMask, 240, 255, fgMask);
    } catch ( cv::Exception &e ) {
        const char *errMsg = e.what();
        cerr << "{ForegroundDetectorMOG2}[ERROR] process(): " << errMsg << endl;
    }
}

void ForegroundDetectorMOG2::draw()
{
    namedWindow("Foreground Mask", CV_WINDOW_AUTOSIZE);
    imshow("Foreground Mask", fgMask);
    waitKey(1);
}
