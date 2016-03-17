#ifndef _H_VS_SOCKET
#define _H_VS_SOCKET

#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <netinet/in.h>
#include <header/VideoStitch.h>

#define SERV_PORT 9527

class VSSocket {

	private:
		struct sockaddr_in mServerAddr;
		int mSocketFD;
		int mArgc;
		char** mArgv;

	public:
		VSSocket(int argc, char** argv);
		void makeConnection();
		std::string waitForString(int bufSize);
		void sendNewImageToClient(cv::Mat img);
};

#endif //_H_VS_SOCKET