#include "VSSocket.h"

VSSocket::VSSocket(int argc, char** argv) : mArgc(argc), mArgv(argv) {
	logMsg(LOG_INFO, "Construct VS Socket");	
}

void VSSocket::makeConnection() {
	if ( (mSocketFD = socket(AF_INET, SOCK_DGRAM, 0) ) < 0 ) {
        logMsg(LOG_ERROR, "Establish socket failed");
        exit(-1);
    }

    bzero ( (char *)&mServerAddr, sizeof(mServerAddr) );
    mServerAddr.sin_family = AF_INET;
    mServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    mServerAddr.sin_port = htons(SERV_PORT);

    if ( bind(mSocketFD, (struct sockaddr *)&mServerAddr, sizeof(mServerAddr) ) < 0) {
        logMsg(LOG_ERROR, "Bind socket failed");
        exit(-2);
    }
}

std::string VSSocket::waitForString(int bufSize) {
	struct sockaddr_in clientAddr;
	int clientAddrSize = sizeof(clientAddr);
	int readBytes;
	std::string rtnStr;

	char* buf = new char[bufSize];
	readBytes = recvfrom(mSocketFD, buf, bufSize, 0, (struct sockaddr*)&clientAddr, (socklen_t *)&clientAddrSize) ;
	rtnStr = string(buf);

	delete[] buf;

	return rtnStr;
}

void VSSocket::sendNewImageToClient(cv::Mat img) {
    // Make it continuous
    img = img.reshape(0, 1);
    int imgSize = img.total() * img.elemSize();

    logMsg(LOG_DEBUG, stringFormat("Plan to send image which size is: %d bytes", imgSize) );

    int rtnValue = sendto(mSocketFD, img.data, imgSize, 0, (struct sockaddr*)&mServerAddr, sizeof(mServerAddr));
    if (rtnValue < 0) {
    	perror("Failed to send update image to client");
        //logMsg(LOG_ERROR, "Failed to send update image to client");
    }
}