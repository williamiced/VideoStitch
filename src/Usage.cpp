#include "header/Usage.h"

void exitWithMsg(returnValEnum errVal, string msg) {
	cout << "Error code: " << errVal << " [" << returnValToString.at(errVal) << "] " << msg << endl;
	exit(errVal);
}

void logMsg(logTypeEnum type, string msg) {
	if (type == LOG_INFO) 
		cout << "[  INFO ] " << msg << endl;
	else if (type == LOG_WARNING)
		cout << "[WARNING] " << msg << endl;
	else if (type == LOG_ERROR)
		cout << "[ ERROR ] " << msg << endl;
	else 
		cerr << "[ DEBUG ] " << msg << endl;
}