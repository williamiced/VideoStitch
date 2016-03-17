#ifndef _H_MANUAL_FEATURE_GENERATOR
#define _H_MANUAL_FEATURE_GENERATOR

#define FILE_NAME "FeatureInfo.txt"
#define WINDOW_NAME "Manual Feature Generator"
#define WINDOW_HEIGHT 540
#define WINDOW_WIDTH 1920

#include <memory>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <time.h>
#include <vector>
#include <header/Usage.h>
#include "opencv2/core/core.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/cuda.hpp"

using namespace std;
using namespace cv;

class ManualFeatureGenerator {
	private:
		int mImageCount;
		vector<Mat> mRawImages;
		int mViewIdx1;
		int mViewIdx2;
		Mat mShowedImg;
		bool mIsWindowActive;
		vector<Scalar> mColorBuffer;
		vector<Point> mLeftBuffer;
		vector<Point> mRightBuffer;
		
		int loadImages(char* fileName);
		Scalar genRandomScalar();
		void redrawWindow();
		void clearBuffer();
		void writeToFile();

	public:
		ManualFeatureGenerator(char* fileName);
		void runMarkProcess();
		bool isWindowActive();
		void leftBtn(int x, int y);
		void rightBtn(int x, int y);
		void middleBtn(int x, int y);
};

unique_ptr<ManualFeatureGenerator> gBC;

#endif //_H_Manual_FEATURE_GENERATOR