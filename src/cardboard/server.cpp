#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include "opencv2/core/core.hpp"
#include "VSSocket.h"
#include <header/VideoStitch.h>

std::unique_ptr<VSSocket> gVss;

void updateImage(Mat img) {
    gVss->sendNewImageToClient(img);
}

int main(int argc, char *argv[]) {
	signal(SIGSEGV, segFaultHandler);

	gVss = std::unique_ptr<VSSocket> ( new VSSocket(argc, argv) );
	gVss->makeConnection();

	while (true) {
		std::string str = gVss->waitForString(1024);
		logMsg(LOG_DEBUG, "Get String: " + str);

		if (str.compare("VideoStitcher") == 0) {
			std::unique_ptr<VideoStitcher> vs ( new VideoStitcher(argc, argv) );
			vs->registerCallbackFunc ( updateImage );
			vs->doRealTimeStitching(argc, argv);
    		vs.release();
		}
	}
}

