#ifndef FOREGROUNDDETECTOR_H
#define FOREGROUNDDETECTOR_H

#include "ImageProcessor.h"
#include "opencv2/video/background_segm.hpp"
#include "opencv2/bgsegm.hpp"

using namespace cv;
using namespace cv::bgsegm;

//---------------------------------------------------------------------------
//  class ImageDifference
//      Foreground region detection by image difference
//---------------------------------------------------------------------------
class ImageDifference : public ForegroundDetector
{
private:
    Mat frame, gray;
    Mat fgMask, bgImage;

public:
    ImageDifference(void);
    ~ImageDifference(void);

    void process(Mat &img);

    void draw();

    Mat& getResult()
    {
        return fgMask;
    }

    Mat& getBackgroundImage()
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
    Mat frame, fgMask, bgImage;
    Ptr<BackgroundSubtractorMOG> bgSubtractor;

public:
    ForegroundDetectorMOG(void);
    ~ForegroundDetectorMOG(void);

    void process(Mat &img);

    void draw();

    Mat& getResult()
    {
        return fgMask;
    }

    Mat& getBackgroundImage();
};

//---------------------------------------------------------------------------
//  class ForegroundDetectorMOG2
//      Foreground region detection by BGS MOG2 (OpenCV)
//---------------------------------------------------------------------------
class ForegroundDetectorMOG2 : public ForegroundDetector
{
private:
    Mat frame, fgMask, bgImage;
    Ptr<BackgroundSubtractorMOG2> bgSubtractor;

public:
    ForegroundDetectorMOG2(void);
    ~ForegroundDetectorMOG2(void);

    void process(Mat &img);

    void draw();

    Mat& getResult()
    {
        return fgMask;
    }

    Mat& getBackgroundImage();
};

#endif // FOREGROUNDDETECTOR_H
