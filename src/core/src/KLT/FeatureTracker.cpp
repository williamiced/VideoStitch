#include "FeatureTracker.h"

FeatureTracker::FeatureTracker(Point point) {
    //ctor
    points.push_back(point);
    drawColor = Scalar(rand() % 256, rand() % 256, rand() % 256);
    isBadFeature = false;
    badFrameCount = 0;
    virtualTraj.clear();
}

FeatureTracker::~FeatureTracker() {
    //dtor
}

void FeatureTracker::move(Point newPoint) {
    points.push_back(newPoint);
}

void FeatureTracker::draw(Mat &canvas, Scalar& color)
{
    // Draw the trajectory of moving point
    int lineThickness = 1;
    //int count = 0;
    for ( size_t i = points.size()-1; i > 0; i-- ) {
        Point2d p = points[i-1];
        Point2d q = points[i];

        circle( canvas, p, 10, color, -1, 8 );
        line( canvas, p, q, color, lineThickness/*, CV_AA, 0*/ );
    }
}

vector<Point> FeatureTracker::getPoints() {
	return points;
}

Scalar& FeatureTracker::getColor() {
	return drawColor;
}

void FeatureTracker::setLastFrameIndex(int index) {
	lastFrameIndex = index;
}

int FeatureTracker::getLastFrameIndex() {
	return lastFrameIndex;
}
