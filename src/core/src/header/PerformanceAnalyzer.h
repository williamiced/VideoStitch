#ifndef _H_PERFORMANCE_ANALYZER
#define _H_PERFORMANCE_ANALYZER

#include <header/Params.h>
#include <header/Usage.h>
#include "opencv2/core/core.hpp"

class PerformanceAnalyzer {
	private:
		int mFrameProcessed;
		vector<double> mExecTimes;

	public:
		void increaseFrame();
		void addExecTime(double time);
		float checkFPS();

		PerformanceAnalyzer();
		~PerformanceAnalyzer();
};

#endif // _H_PERFORMANCE_ANALYZER
