#ifndef VIDEOVOLUMEANALYZER_H
#define VIDEOVOLUMEANALYZER_H

#include <opencv2/opencv.hpp>
#include <algorithm>
#include <vector>
#include <list>
#include <opencv2/xfeatures2d.hpp>

#include "FeatureTracker.h"
#include "ForegroundDetector.h"

using namespace cv;
using std::vector;
using std::list;
using namespace cv::xfeatures2d;

class VideoVolumeAnalyzer
{
public:
    /// Default constructor
    VideoVolumeAnalyzer();

    /// Default destructor
    virtual ~VideoVolumeAnalyzer();

    /// process an incoming frame
    void process(Mat &frame);

    void draw();

    list<FeatureTracker*> getActiveTrackers();
    Mat& getImage();

protected:
    list<FeatureTracker*> activeTrackers;
    list<FeatureTracker*> deadTrackers;
    void trackFeatures(Rect& boundary);
    void trackFeaturesICCV09();

private:
    int mFrameCounter;
    ForegroundDetector *fgDetector;
    Mat image, fgMask;
    Mat prevGray, currGray;

    void deleteTrackers();
};

#endif // VIDEOVOLUMEANALYZER_H
