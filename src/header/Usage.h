#ifndef _H_USAGE
#define _H_USAGE

#include <iostream>
#include <string>
#include <boost/assign/list_of.hpp>
#include <boost/unordered_map.hpp>

using namespace std;
using boost::assign::map_list_of;

enum returnValEnum {
	N_NORMAL, E_BAD_ARGUMENTS, E_FILE_NOT_EXISTS
};

enum logTypeEnum {
	LOG_INFO, LOG_WARNING, LOG_ERROR, LOG_DEBUG
};

const boost::unordered_map<returnValEnum, const char*> returnValToString = map_list_of
    (N_NORMAL, "Successfully executes.")
    (E_BAD_ARGUMENTS, "Bad arguments. Please check the usage.")
    (E_FILE_NOT_EXISTS, "Files or directories cannot be found.");

void exitWithMsg(returnValEnum errVal, string msg = NULL);
void logMsg(logTypeEnum type, string msg);

#endif // _H_USAGE