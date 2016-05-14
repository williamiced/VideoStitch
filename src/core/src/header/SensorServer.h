#ifndef _H_SENSOR_SERVER
#define _H_SENSOR_SERVER

#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <iterator>
#include <netinet/in.h>
#include <header/Usage.h>
#include <header/Params.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <header/Usage.h>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#define SERV_PORT 9527
#define BUF_SIZE 1024

using namespace std;
using namespace cv;
using namespace boost::algorithm;

class SensorServer {
	private:
		struct sockaddr_in mServerAddr;
		string mClientIP;
		int mSocketFD;
		float* mOrientation;
		bool mIsSensorWorks;
		thread mServerThread;
		int mW, mH;
		float mFovealDiameter;
		bool mIsRealTimeStreaming;
		vector<vector<float> > mSensorDataVec;
		vector<vector<float> >::iterator mSensorIter;

		void loadFromFile(string fileName);
		void makeConnection();
		void parseSensorInfo(char* buf);
		void saveClientIP(char* ip);

	public:
		SensorServer();
		SensorServer(string fileName);
		void getFovealInfo(int& renderDiameter, Point2f& renderCenter);
		void getRenderArea(Rect& area, Mat& mask);
		bool isSensorWorks();
		string getClientIP();
};

#endif //_H_SENSOR_SERVER