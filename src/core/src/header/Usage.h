#ifndef _H_USAGE
#define _H_USAGE

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <ctime> 
#include <map>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>  // For va_start, etc.
#include <memory>    // For std::unique_ptr
#include <boost/assign/list_of.hpp>
#include <boost/unordered_map.hpp>
#include <boost/current_function.hpp>
#include <boost/timer/timer.hpp>
#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"

#ifdef CARE_TIME
	#define SETUP_TIMER cout << __FUNCTION__ << endl;boost::timer::auto_cpu_timer boostTimer;
#else
    #define SETUP_TIMER ;
#endif

#define unsafe(i)  \
        ( (i) >= 0 ? (i) : -(i) )

using namespace cv;
using namespace std;
using boost::assign::map_list_of;

enum angle{ YAW = 0 , ROLL = 1 , PITCH = 2 };
enum direction{ X = 0 , Y = 1 , Z = 2 };

enum returnValEnum {
	N_NORMAL, E_BAD_ARGUMENTS, E_FILE_NOT_EXISTS, E_TOO_FEW_VIDEOS, E_RUNTIME_ERROR
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
    (E_TOO_FEW_VIDEOS, "Loaded videos are too few to stitch.")
    (E_RUNTIME_ERROR, "Run time error");

extern map<string, string> CONFIG;

char* getCmdOption(char** begin, char** end, const std::string & option);
bool cmdOptionExists(char** begin, char** end, const std::string& option);
bool checkArguments(int argc, char** argv);

int getIntConfig(string name);
float getFloatConfig(string name);
string getStringConfig(string name);

void loadConfig(char* filename);
void exitWithMsg(returnValEnum errVal, string msg = NULL);
void logMsg(logTypeEnum type, string msg);
void logMsg(logTypeEnum type, string msg, int threadIdx);
string stringFormat(const string fmt_str, ...); 
Mat getRotationMatrix(double yaw, double pitch, double roll);
Mat getZMatrix(double alpha);
Mat getYMatrix(double beta);
Mat getXMatrix(double gamma);
void segFaultHandler (int sig);

class RMat{
public:
	double get_yaw(){ return eulerangle[YAW]; }
	double get_roll(){ return eulerangle[ROLL]; }
	double get_pitch(){ return eulerangle[PITCH]; }
	Mat getR() { return getRotationMatrix(eulerangle[YAW], eulerangle[PITCH], eulerangle[ROLL] ); }

	void set(double yaw, double pitch, double roll) { eulerangle[YAW] = yaw; eulerangle[PITCH] = pitch; eulerangle[ROLL] = roll;}
	void set_yaw(double temp_yaw){ eulerangle[YAW] = temp_yaw; }
	void set_roll(double temp_roll){ eulerangle[ROLL] = temp_roll; }
	void set_pitch(double temp_pitch){ eulerangle[PITCH] = temp_pitch; }

private:
	double eulerangle[3];
};

class TMat{
public:
	double get_x(){ return direction[X]; }
	double get_y(){ return direction[Y]; }
	double get_z(){ return direction[Z]; }

	void set(double x, double y, double z) { direction[X] = x; direction[Y] = y; direction[Z] = z;}
	void set_x(double temp_x){ direction[X] = temp_x; }
	void set_y(double temp_y){ direction[Y] = temp_y; }
	void set_z(double temp_z){ direction[Z] = temp_z; }

private:
	double direction[3];
};

class ExtrinsicParam {
public:
	ExtrinsicParam();
	ExtrinsicParam(double y, double p, double r, double tx, double ty, double tz);

	Mat getR() { return R.getR(); }
	double get_RMat_Value(int angle);
	void set_RMat_Value(int angle , double value);
	double get_TMat_Value(int direction);
	void set_TMat_Value(int direction , double value);

private:
	RMat R;
	TMat T;
};

class ExtrinsicParamSet {
public:
	ExtrinsicParamSet(int num_camera);
	void generatePerturbation(int num_random, vector<ExtrinsicParamSet>& candidatePool);
	void setParam(int index, ExtrinsicParam param);
	ExtrinsicParamSet clone();

	vector<ExtrinsicParam> params;
};

class FeatureMatch {
public:
	Point p1;
	Point p2;
};

class MatchInfo {
public:
	int idx1;
	int idx2;
	vector<FeatureMatch> matches;
};


#endif // _H_USAGE