#include <header/SensorServer.h>

SensorServer::SensorServer() : mOrientation(new float[3]), mIsSensorWorks(false) {
    mServerThread = thread(&SensorServer::makeConnection, this);
}

void SensorServer::makeConnection() {
	if ( (mSocketFD = socket(AF_INET, SOCK_DGRAM, 0) ) < 0 ) {
        logMsg(LOG_ERROR, "Establish socket failed", 3);
        exit(-1);
    }

    bzero ( (char *)&mServerAddr, sizeof(mServerAddr) );
    mServerAddr.sin_family = AF_INET;
    mServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    mServerAddr.sin_port = htons(SERV_PORT);

    if ( bind(mSocketFD, (struct sockaddr *)&mServerAddr, sizeof(mServerAddr) ) < 0) {
        logMsg(LOG_ERROR, "Bind socket failed", 3);
        exit(-2);
    }

    struct sockaddr_in clientAddr;
    int clientAddrSize = sizeof(clientAddr);
    int readBytes;

    char* buf = new char[BUF_SIZE];

    logMsg(LOG_INFO, "Sensor connection loop START!!", 3);

    while (true) {
        readBytes = recvfrom(mSocketFD, buf, BUF_SIZE, 0, (struct sockaddr*)&clientAddr, (socklen_t *)&clientAddrSize) ;

        if (readBytes <= 0)
            break;
        if (!mIsSensorWorks)
            saveClientIP(inet_ntoa(clientAddr.sin_addr));
        mIsSensorWorks = true;
        parseSensorInfo(buf);
    }
    logMsg(LOG_INFO, "Sensor connection loop END!!", 3);    

    delete[] buf;
}

void SensorServer::saveClientIP(char* ip) {
    mClientIP = string(ip);
}

string SensorServer::getClientIP() {
    return mClientIP;
}

bool SensorServer::isSensorWorks() {
    return mIsSensorWorks;
}

void SensorServer::parseSensorInfo(char* buf) {
    //vector<string> ans;
    char *src = strdup(buf);
    char *ptr = strtok(src, ","); 
    int counter = 0;
    while (ptr != NULL && counter < 3) {
        mOrientation[counter] = atof(ptr);
        counter++;
        ptr = strtok(NULL, ","); 
    }
    //logMsg(LOG_DEBUG, stringFormat("Orientation Updated: %f, %f, %f", mOrientation[0], mOrientation[1], mOrientation[2]));
}

void SensorServer::getRenderArea(Rect& area, Mat& mask) {
    float centerU = (mOrientation[0] + M_PI) / (2 * M_PI);
    float centerV = (M_PI - fabs(mOrientation[2]) ) / M_PI;
    float uOffset = (55.f / 360.f);
    float vOffset = (55.f / 180.f);

    float u0 = centerU - uOffset;
    float u1 = centerU + uOffset;
    float v0 = centerV - vOffset;
    float v1 = centerV + vOffset;
    
    if (u0 < 0 || u1 > 1) {
        u0 = 0.f;
        u1 = 1.f;
    } 
    if (v0 < 0 || v1 > 1) {
        v0 = 0.f;
        v1 = 1.f;
    }
    
    // debug
    /*
    u0 = 0.f;
    u1 = 1.f;
    v0 = 0.f;
    v1 = 1.f;
    */

    area = Rect(u0 * OUTPUT_PANO_WIDTH, v0 * OUTPUT_PANO_HEIGHT, (u1-u0) * OUTPUT_PANO_WIDTH, (v1-v0) * OUTPUT_PANO_HEIGHT);
    mask = Mat((v1-v0) * OUTPUT_PANO_HEIGHT, (u1-u0) * OUTPUT_PANO_WIDTH, CV_8UC1, 1);
}