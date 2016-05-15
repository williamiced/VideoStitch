#ifndef _H_TOOL_BOX_PARSER
#define _H_TOOL_BOX_PARSER

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

#include <header/Usage.h>

#include <boost/tokenizer.hpp>

#include "opencv2/core/core.hpp"
#include "opencv2/opencv.hpp"

using namespace std;

class ToolBoxParser {
	private:

	public:
		// Intrinsic parameters
		static vector< float > mX; // xi
		static vector< cv::Point2f > mF; // Focal length
		static vector< float > mAr; // Aspect ratio
		static vector< cv::Point > mC; // Principle point
		static vector< cv::Mat > mD; // Distortion coefficients
		static vector< cv::Mat > mK; // Intrinsic matrix

		// Extrinsic parameters
		static vector< cv::Mat > mR; // Rotation matrix
		static vector< cv::Mat > mT; // Translation matrix

		static vector<float> parseIntrinsicInfo(string lineStr);
		static vector<float> parseExtrinsicInfo(string valueStr);
		static string removeSpaces(string input);
		static string removeBrackets(string input);
		static void parseToolBoxFile(char* fileName);
	
};

#endif //_H_TOOL_BOX_PARSER