#include <header/PerformanceAnalyzer.h>

void PerformanceAnalyzer::increaseFrame() {
	mFrameProcessed++;
}

void PerformanceAnalyzer::addExecTime(double time) {
	mExecTimes.push_back( time );
}

float PerformanceAnalyzer::checkFPS() {
	double total = 0.f;
	for (unsigned int i=0; i<mExecTimes.size(); i++) 
		total += mExecTimes[i];
	logMsg(LOG_INFO, stringFormat( "=== Average FPS is %lf === ", mFrameProcessed / total ) );
	return mFrameProcessed / total;
}

PerformanceAnalyzer::PerformanceAnalyzer() : mFrameProcessed(0) {
}

PerformanceAnalyzer::~PerformanceAnalyzer() {
	
}