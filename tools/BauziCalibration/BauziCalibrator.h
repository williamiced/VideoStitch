#ifndef _H_BAUZI_CALIBRATOR
#define _H_BAUZI_CALIBRATOR

#include <stdio.h>
#include <string.h>
#include <memory>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <header/Usage.h>
#include "opencv2/core/core.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/xfeatures2d.hpp"
#include "opencv2/stitching/detail/matchers.hpp"
#include "opencv2/stitching/detail/camera.hpp"
#include "opencv2/stitching/detail/motion_estimators.hpp"
#include "opencv2/core/cuda.hpp"
#include "opencv2/stitching/warpers.hpp"

using namespace std::placeholders;
using namespace std;
using namespace cv;
using namespace cv::xfeatures2d;

class BauziCalibrator {
	private:
		int mImageCount;
		vector<Mat> mRawImages;
		unique_ptr<ExtrinsicParamSet> mEPSet;
		vector<MatchInfo> mMatchInfos;
		unique_ptr<cv::detail::SphericalWarper> mSW;
		vector<Mat> mK;

		int loadImages(char* fileName);
		void genInitGuess();
		void findBestParams(int iterationCount);
		void evaluateError(vector<ExtrinsicParamSet> possibleSets, vector<double>& scores);
		double getScore(ExtrinsicParamSet eps);
		void chooseCandidateSet(vector<ExtrinsicParamSet> possibleSets, vector<double> scores, vector<ExtrinsicParamSet>& candidateSet, int chooseAmount);
		void doFeatureMatching();

	public:
		void loadFeatureInfoFromFile(char* fileName);
		void runProcess(int iterationCount);
		BauziCalibrator(char* fileName);
};

#endif //_H_BAUZI_CALIBRATOR