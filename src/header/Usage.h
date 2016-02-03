#ifndef _H_USAGE
#define _H_USAGE

#include <iostream>
#include <string>
#include <ctime> 
#include <stdarg.h>  // For va_start, etc.
#include <memory>    // For std::unique_ptr
#include <boost/assign/list_of.hpp>
#include <boost/unordered_map.hpp>
#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"

using namespace cv;
using namespace std;
using boost::assign::map_list_of;

enum returnValEnum {
	N_NORMAL, E_BAD_ARGUMENTS, E_FILE_NOT_EXISTS, E_TOO_FEW_VIDEOS
};

enum logTypeEnum {
	LOG_INFO, LOG_WARNING, LOG_ERROR, LOG_DEBUG
};

typedef struct RenderArea {
	Rect roi; // ROI
	Mat mask; // mask
} RenderArea;

struct MutualProjectParam {
	double r;
	double p;
	double y;
	double TrX;
	double TrY;
	double TrZ;
};

const boost::unordered_map<returnValEnum, const char*> returnValToString = map_list_of
    (N_NORMAL, "Successfully executes.")
    (E_BAD_ARGUMENTS, "Bad arguments. Please check the usage.")
    (E_FILE_NOT_EXISTS, "Files or directories cannot be found.")
    (E_TOO_FEW_VIDEOS, "Loaded videos are too few to stitch.");
    
void exitWithMsg(returnValEnum errVal, string msg = NULL);
void logMsg(logTypeEnum type, string msg);
string stringFormat(const string fmt_str, ...); 

#endif // _H_USAGE