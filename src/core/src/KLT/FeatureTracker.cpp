#include "FeatureTracker.h"

FeatureTracker::FeatureTracker(Point point) {
    //ctor
    points.push_back(point);
    //drawColor = Scalar(rand() % 256, rand() % 256, rand() % 256);
    drawColor = Scalar(0, 0, 255);
}

FeatureTracker::~FeatureTracker() {
    //dtor
}

void FeatureTracker::move(Point newPoint) {
    points.push_back(newPoint);
}

void FeatureTracker::draw(Mat &canvas, Scalar& color) {
    // Draw the trajectory of moving point
    int lineThickness = 1;

    circle( canvas, points.back(), 2, color, -1, 8 );

    for ( size_t i = 0; i < points.size() - 1; i++ ) {
        Point2d p = points[i];
        Point2d q = points[i+1];

        line( canvas, p, q, Scalar(0, 0, 0), lineThickness);
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
