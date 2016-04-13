#ifndef _H_SENSOR_SERVER
#define _H_SENSOR_SERVER

#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <thread>
#include <netinet/in.h>
#include <header/Usage.h>
#include <header/Params.h>

#define SERV_PORT 9527
#define BUF_SIZE 1024

using namespace std;
using namespace cv;

class SensorServer {
	private:
		struct sockaddr_in mServerAddr;
		int mSocketFD;
		float* mOrientation;
		bool mIsSensorWorks;
		thread mServerThread;

		void makeConnection();
		void parseSensorInfo(char* buf);

	public:
		SensorServer();
		void getRenderArea(Rect& area, Mat& mask);
		bool isSensorWorks();
};

#endif //_H_SENSOR_SERVER