#ifndef FEATURETRACKER_H
#define FEATURETRACKER_H

#define TRAJECTORIES_DRAW_THRESHOLD 30

#include <opencv2/opencv.hpp>
#include <vector>

using namespace cv;
using std::vector;

class FeatureTracker
{
public:
    /** Default constructor */
    FeatureTracker(Point point);

    /** Default destructor */
    virtual ~FeatureTracker();

    Point getLastPoint() const
    {
        return points.back();
    }

    int length() const
    {
        return points.size();
    }

    void removeLastPoint() 
    {
        points.pop_back();
    }

    void move(Point newPoint);

    void draw(Mat &canvas, Scalar& color);
    
    vector<Point> getPoints();
    Scalar& getColor();
    void setLastFrameIndex(int index);
    int getLastFrameIndex();

    bool isAlreadyDie;
    bool isBadFeature;
    int badFrameCount;
    vector<Point2f> virtualTraj;

protected:

private:
    // Past positions of moving point
    vector<Point> points;
    Scalar drawColor;
    int lastFrameIndex;
};

#endif // FEATURETRACKER_H
