#include <header/SensorServer.h>

SensorServer::SensorServer() : mOrientation(new float[3]), mIsSensorWorks(false), mW(getIntConfig("OUTPUT_PANO_WIDTH")), mH(getIntConfig("OUTPUT_PANO_HEIGHT")), mFovealDiameter(-1) {
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
        readBytes = recvfrom(mSocketFD, buf, BUF_SIZE, 0, (struct sockaddr*)&clientAddr, (socklen_t *)&clientAddrSize);
        //int result = sendto(mSocketFD, buf, BUF_SIZE, 0, (struct sockaddr*)&clientAddr, clientAddrSize);

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

    float u0, u1, v0, v1;
    
    /*
    if (mIsSensorWorks) {
        u0 = centerU - uOffset;
        u1 = centerU + uOffset;
        v0 = centerV - vOffset;
        v1 = centerV + vOffset;

        if (u0 < 0 || u1 > 1) {
            u0 = 0.f;
            u1 = 1.f;
        } 
        if (v0 < 0 || v1 > 1) {
            v0 = 0.f;
            v1 = 1.f;
        }
    } else {
        // debug
        u0 = 0.f;
        u1 = 1.f;
        v0 = 0.f;
        v1 = 1.f;    
    }
    */
    u0 = 0.f;
    u1 = 1.f;
    v0 = 0.f;
    v1 = 1.f;    
    
    area = Rect(u0 * mW, v0 * mH, (u1-u0) * mW, (v1-v0) * mH);
    mask = Mat((v1-v0) * mH, (u1-u0) * mW, CV_8UC1, 1);
}

void SensorServer::getFovealInfo(int& renderDiameter, Point2f& renderCenter) {
    // 4320: 1200
    // 2160: 600
    // 1440: 400
    if (mFovealDiameter < 0) {
        float pixelDensity = (getIntConfig("OUTPUT_PANO_WIDTH") / 3.6f)  / 138.5f;     // px/mm
        float userDist = 100.f;                // mm
        float worstLatency = 150.f;             // ms
        float maxSaccadicSpeed = (0.2f / 180.f) * M_PI;
        float subtendedAngle = (5.f / 180.f) * M_PI;
        float blendingBorder = 10.f;            
        float errorConstant = 0.f;

        mFovealDiameter = static_cast<int>(2.f * pixelDensity * userDist * tan(worstLatency * maxSaccadicSpeed + subtendedAngle / 2.f) + 2.f * blendingBorder + errorConstant);
    }
    renderDiameter = mFovealDiameter;
    float centerU = (mOrientation[0] + M_PI) / (2 * M_PI);
    float centerV = (M_PI - fabs(mOrientation[2]) ) / M_PI;

    renderCenter = Point2f(centerU, centerV);
}