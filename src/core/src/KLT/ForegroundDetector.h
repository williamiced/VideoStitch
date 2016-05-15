#ifndef FOREGROUNDDETECTOR_H
#define FOREGROUNDDETECTOR_H

#include "ImageProcessor.h"
#include "opencv2/video/background_segm.hpp"
#include "opencv2/bgsegm.hpp"

using namespace cv::bgsegm;

//---------------------------------------------------------------------------
//  class ImageDifference
//      Foreground region detection by image difference
//---------------------------------------------------------------------------
class ImageDifference : public ForegroundDetector
{
private:
    cv::Mat frame, gray;
    cv::Mat fgMask, bgImage;

public:
    ImageDifference(void);
    ~ImageDifference(void);

    void process(cv::Mat &img);

    void draw();

    cv::Mat& getResult()
    {
        return fgMask;
    }

    cv::Mat& getBackgroundImage()
    {
        return bgImage;
    };
};

//---------------------------------------------------------------------------
//  class ForegroundDetectorMOG
//      Foreground region detection by BGS MOG (OpenCV)
//---------------------------------------------------------------------------
class ForegroundDetectorMOG : public ForegroundDetector
{
private:
    cv::Mat frame, fgMask, bgImage;
    cv::Ptr<cv::bgsegm::BackgroundSubtractorMOG> bgSubtractor;

public:
    ForegroundDetectorMOG(void);
    ~ForegroundDetectorMOG(void);

    void process(cv::Mat &img);

    void draw();

    cv::Mat& getResult()
    {
        return fgMask;
    }

    cv::Mat& getBackgroundImage();
};

//---------------------------------------------------------------------------
//  class ForegroundDetectorMOG2
//      Foreground region detection by BGS MOG2 (OpenCV)
//---------------------------------------------------------------------------
class ForegroundDetectorMOG2 : public ForegroundDetector
{
private:
    cv::Mat frame, fgMask, bgImage;
    cv::Ptr<cv::BackgroundSubtractorMOG2> bgSubtractor;

public:
    ForegroundDetectorMOG2(void);
    ~ForegroundDetectorMOG2(void);

    void process(cv::Mat &img);

    void draw();

    cv::Mat& getResult()
    {
        return fgMask;
    }

    cv::Mat& getBackgroundImage();
};

#endif // FOREGROUNDDETECTOR_H
