#ifndef FEATURETRACKER_H
#define FEATURETRACKER_H

#define TRAJECTORIES_DRAW_THRESHOLD 30

#include <opencv2/opencv.hpp>
#include <vector>

using std::vector;

class FeatureTracker
{
public:
    /** Default constructor */
    FeatureTracker(cv::Point point);

    /** Default destructor */
    virtual ~FeatureTracker();

    cv::Point getLastPoint() const {
        return points.back();
    }

    int length() const {
        return points.size();
    }

    void removeLastPoint()  {
        points.pop_back();
    }

    void move(cv::Point newPoint);

    void draw(cv::Mat &canvas, cv::Scalar& color);
    
    vector<cv::Point> getPoints();
    cv::Scalar& getColor();
    void setLastFrameIndex(int index);
    int getLastFrameIndex();

protected:

private:
    // Past positions of moving point
    vector<cv::Point> points;
    cv::Scalar drawColor;
    int lastFrameIndex;
};

#endif // FEATURETRACKER_H
