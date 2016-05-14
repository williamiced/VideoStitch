#include <header/Usage.h>

map<string, string> CONFIG;

int getIntConfig(string name) {
    return stoi(CONFIG[name]);
}

float getFloatConfig(string name) {
    return stof(CONFIG[name]);
}

string getStringConfig(string name) {
    if (CONFIG.find(name) == CONFIG.end())
        return "";
    return CONFIG[name];
}

char* getCmdOption(char** begin, char** end, const std::string & option) {
    char ** itr = std::find(begin, end, option);
    if (itr != end && ++itr != end) 
        return *itr;
    return 0;
}

bool cmdOptionExists(char** begin, char** end, const std::string& option) {
    return std::find(begin, end, option) != end;
}

bool checkArguments(int argc, char** argv) {
    if ( !cmdOptionExists(argv, argv + argc, "--input") )
        return false;
    if ( !cmdOptionExists(argv, argv + argc, "--config") )
        return false;
    return true;
}

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

void loadConfig(char* filename) {
    ifstream inputFile(filename);

    string config;
    while (getline(inputFile, config)) {
        if (config.find(":") == 0)
            continue;
        // Parse
        int sep = config.find(":=");
        string key = config.substr(0, sep);
        string val = config.substr(sep+2);

        CONFIG[key] = val;
    }
    logMsg(LOG_INFO, stringFormat("=== Load %d configs ===", CONFIG.size()));
}

void exitWithMsg(returnValEnum errVal, string msg) {
    cout << "Error code: " << errVal << " [" << returnValToString.at(errVal) << "] " << msg << endl;
    exit(errVal);
}

void logMsg(logTypeEnum type, string msg, int threadIdx) {
    string head;
    string tail = "\033[0m";
    if (threadIdx == 1)
        head = "\033[1;34m";
    else if (threadIdx == 2)
        head = "\033[1;32m";
    else if (threadIdx == 3)
        head = "\033[1;35m";
    else if (threadIdx == 4)
        head = "\033[1;36m";
    if (type == LOG_WARNING)
        head = "\033[1;33m";
    else if (type == LOG_ERROR)
        head = "\035[1;31m";
    time_t result = time(nullptr);
    if (type == LOG_INFO) 
        cout << head << "[  INFO ] " << msg << "\t\t\t - " << asctime(localtime(&result)) << tail;
    else if (type == LOG_WARNING)
        cout << head << "[WARNING] " << msg << "\t\t\t - " << asctime(localtime(&result)) << tail;
    else if (type == LOG_ERROR)
        cout << head << "[ ERROR ] " << msg << "\t\t\t - " << asctime(localtime(&result)) << tail;
    else 
        cerr << head << "[ DEBUG ] " << msg << "\t\t\t - " << asctime(localtime(&result)) << tail;
}

void logMsg(logTypeEnum type, string msg) {
    logMsg(type, msg, 0);
}

Mat getRotationMatrix(double yaw, double pitch, double roll) {
    double alpha = yaw;
    double beta = pitch;
    double gamma = roll;

    // Take camera as reference coordinate system, around: x-axis -> pitch, y-axis -> yaw, z->axis -> roll
    Mat Rz = getZMatrix(gamma);
    Mat Ry = getYMatrix(alpha);
    Mat Rx = getXMatrix(beta);
    return Ry * Rx * Rz;
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

void segFaultHandler (int sig) {
  void *array[10];
  size_t size;

  size = backtrace(array, 10);

  logMsg(LOG_ERROR, stringFormat("Error: signal %d:\n", sig));
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}

ExtrinsicParam::ExtrinsicParam() {
    R.set(0.f, 0.f, 0.f);
    T.set(0.f, 0.f, 0.f);
}

ExtrinsicParam::ExtrinsicParam(double y, double p, double r, double tx, double ty, double tz) {
    R.set(y, p, r);
    T.set(tx, ty, tz);
}

double ExtrinsicParam::get_RMat_Value(int angle){ 
    if(angle == YAW){
        return R.get_yaw();
    }
    else if(angle == ROLL){
        return R.get_roll();
    }
    else if(angle == PITCH){
        return R.get_pitch();
    }
    else {
        logMsg(LOG_WARNING, stringFormat("Wrong angle: %d", angle));
        return -1.f;
    }
}

void ExtrinsicParam::set_RMat_Value(int angle , double value){ 
    if(angle == YAW){
        R.set_yaw(value);
    }
    else if(angle == ROLL){
        R.set_roll(value);
    }
    else if(angle == PITCH){
        R.set_pitch(value);
    }
}

double ExtrinsicParam::get_TMat_Value(int direction){ 
    if(direction == X){
        return T.get_x();
    }
    else if(direction == Y){
        return T.get_y();
    }
    else if(direction == Z){
        return T.get_z();
    }
    else {
        logMsg(LOG_WARNING, stringFormat("Wrong direction: %d", direction));
        return -1.f;
    }
}

void ExtrinsicParam::set_TMat_Value(int direction , double value){ 
    if(direction == X){
        T.set_x(value);
    }
    else if(direction == Y){
        T.set_y(value);
    }
    else if(direction == Z){
        T.set_z(value);
    }
}

ExtrinsicParamSet::ExtrinsicParamSet(int num_camera) {
    ExtrinsicParam temp;
    for(int i=0 ; i < num_camera ; i+=1){
        params.push_back(temp);
    }
}

void ExtrinsicParamSet::setParam(int index, ExtrinsicParam param) {
    params[index] = param;
}

void ExtrinsicParamSet::generatePerturbation(int num_random, vector<ExtrinsicParamSet>& candidatePool) {
    for(int i = 0 ; i < num_random ; i +=1){
        int rand_angle = (int)( rand() % 3 );
        int rand_direction = (int)( rand() % 3 );
        int rand_choose = (int)( rand() % params.size() );
        double rand_angle_value = rand() / 1000000000000.0;
        double rand_shift_value = rand() / 1000000000000.0;

        ExtrinsicParamSet cloneEP = clone();
        ExtrinsicParam temp = params[rand_choose];
        temp.set_RMat_Value(rand_angle , temp.get_RMat_Value(rand_angle) + rand_angle_value);
        temp.set_TMat_Value(rand_direction , temp.get_TMat_Value(rand_direction) + rand_shift_value);
        cloneEP.setParam(rand_choose, temp);

        candidatePool.push_back(cloneEP);
    }

    logMsg(LOG_DEBUG, stringFormat("Generate %d perturbation", candidatePool.size()));
}

ExtrinsicParamSet ExtrinsicParamSet::clone() {
    int size = params.size();
    ExtrinsicParamSet cloneSet(size);
    for (int i=0; i<size; i++) {
        cloneSet.setParam(i, params[i]);
    }
    return cloneSet;
}