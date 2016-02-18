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
using namespace cv;

class ToolBoxParser {
	private:

	public:
		// Intrinsic parameters
		static vector< float > mX; // xi
		static vector< Point2f > mF; // Focal length
		static vector< float > mAr; // Aspect ratio
		static vector< Point > mC; // Principle point
		static vector< Mat > mD; // Distortion coefficients
		static vector< Mat > mK; // Intrinsic matrix

		// Extrinsic parameters
		static vector< Mat > mR; // Rotation matrix
		static vector< Mat > mT; // Translation matrix

		static vector<float> parseIntrinsicInfo(string lineStr);
		static vector<float> parseExtrinsicInfo(string valueStr);
		static string removeSpaces(string input);
		static string removeBrackets(string input);
		static void parseToolBoxFile(char* fileName);
	
};

#endif //_H_TOOL_BOX_PARSER