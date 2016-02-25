#include <header/Usage.h>

string stringFormat(const string fmt_str, ...) {
    int final_n, n = ((int)fmt_str.size()) * 2; /* Reserve two times as much as the length of the fmt_str */
    string str;
    unique_ptr<char[]> formatted;
    va_list ap;
    while(1) {
        formatted.reset(new char[n]); /* Wrap the plain char array into the unique_ptr */
        strcpy(&formatted[0], fmt_str.c_str());
        va_start(ap, fmt_str);
        final_n = vsnprintf(&formatted[0], n, fmt_str.c_str(), ap);
        va_end(ap);
        if (final_n < 0 || final_n >= n)
            n += abs(final_n - n + 1);
        else
            break;
    }
    return string(formatted.get());
}

void exitWithMsg(returnValEnum errVal, string msg) {
    cout << "Error code: " << errVal << " [" << returnValToString.at(errVal) << "] " << msg << endl;
    exit(errVal);
}

void logMsg(logTypeEnum type, string msg) {
    time_t result = time(nullptr);
    if (type == LOG_INFO) 
        cout << "[  INFO ] " << msg << "\t\t\t - " << asctime(localtime(&result));
    else if (type == LOG_WARNING)
        cout << "[WARNING] " << msg << "\t\t\t - " << asctime(localtime(&result));
    else if (type == LOG_ERROR)
        cout << "[ ERROR ] " << msg << "\t\t\t - " << asctime(localtime(&result));
    else 
        cerr << "[ DEBUG ] " << msg << "\t\t\t - " << asctime(localtime(&result));
}

Mat getZMatrix(double alpha) {
    Mat z = Mat::zeros(3, 3, CV_32F);
    float cosz = cos(alpha);
    float sinz = sin(alpha);
    z.at<float>(0, 0) = cosz;
    z.at<float>(0, 1) = -sinz;
    z.at<float>(1, 0) = sinz;
    z.at<float>(1, 1) = cosz;
    z.at<float>(2, 2) = 1.f;
    return z;
}

Mat getYMatrix(double beta) {
    Mat y = Mat::zeros(3, 3, CV_32F);
    float cosy = cos(beta);
    float siny = sin(beta);
    y.at<float>(0, 0) = cosy;
    y.at<float>(0, 2) = siny;
    y.at<float>(1, 1) = 1.f;
    y.at<float>(2, 0) = -siny;
    y.at<float>(2, 2) = cosy;
    return y;
}

Mat getXMatrix(double gamma) {
    Mat x = Mat::zeros(3, 3, CV_32F);
    float cosx = cos(gamma);
    float sinx = sin(gamma);
    x.at<float>(0, 0) = 1.f;
    x.at<float>(1, 1) = cosx;
    x.at<float>(1, 2) = -sinx;
    x.at<float>(2, 1) = sinx;
    x.at<float>(2, 2) = cosx;
    return x;
}
