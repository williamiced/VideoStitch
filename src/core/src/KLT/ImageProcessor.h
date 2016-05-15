#ifndef IMAGEPROCESSOR_H_INCLUDED
#define IMAGEPROCESSOR_H_INCLUDED

#include <opencv2/opencv.hpp>

//--------------------------------------------------------------------------
//  Base class : ImageProcessor
//      Abstract class for generic image processors using
//      different algorithms
//--------------------------------------------------------------------------
class ImageProcessor
{
public:
    // process image
    virtual void process(cv::Mat &img) = 0;

    // draw processed image
    virtual void draw() = 0;

    virtual cv::Mat& getResult() = 0;

    virtual ~ImageProcessor(void) {};
};

//--------------------------------------------------------------------------
//  Base class : ForegroundDetector
//      Abstract class for generic image processors for generating a
//      foreground mask by using different BGS based algorithms.
//--------------------------------------------------------------------------
class ForegroundDetector : public ImageProcessor
{
protected:
    bool updateFlag;

public:
    // Retrieve the background image maintained by the underlying algorithm.
    virtual cv::Mat& getBackgroundImage() = 0;

    // Turn on or off background model updating
    void toggleUpdate(bool flag)
    {
        updateFlag = flag;
    };

    // NOTE: For each derived class, override process() virtual function
    // to obtain a foreground mask.
    //virtual void process(Mat &img) = 0;

    // NOTE: For each derived class, override getResult() virtual function
    // to return the computed foreground mask.
    //virtual Mat& getResult() = 0;

    virtual ~ForegroundDetector(void) {};
};

#endif // IMAGEPROCESSOR_H_INCLUDED
