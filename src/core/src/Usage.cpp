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

string testStr() {
    return string("HelloWorld");
}