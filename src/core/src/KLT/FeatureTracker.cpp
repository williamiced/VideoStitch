#include "FeatureTracker.h"

FeatureTracker::FeatureTracker(cv::Point point) {
    //ctor
    points.push_back(point);
    //drawColor = Scalar(rand() % 256, rand() % 256, rand() % 256);
    drawColor = cv::Scalar(0, 0, 255);
}

FeatureTracker::~FeatureTracker() {
    //dtor
}

void FeatureTracker::move(cv::Point newPoint) {
    points.push_back(newPoint);
}

void FeatureTracker::draw(cv::Mat &canvas, cv::Scalar& color) {
    // Draw the trajectory of moving point
    int lineThickness = 1;

    cv::circle( canvas, points.back(), 2, color, -1, 8 );

    for ( size_t i = 0; i < points.size() - 1; i++ ) {
        cv::Point2d p = points[i];
        cv::Point2d q = points[i+1];

        cv::line( canvas, p, q, cv::Scalar(0, 0, 0), lineThickness);
    }
}

vector<cv::Point> FeatureTracker::getPoints() {
	return points;
}

cv::Scalar& FeatureTracker::getColor() {
	return drawColor;
}

void FeatureTracker::setLastFrameIndex(int index) {
	lastFrameIndex = index;
}

int FeatureTracker::getLastFrameIndex() {
	return lastFrameIndex;
}
