#include <header/VideoStitch.h>

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
	if (!cmdOptionExists(argv, argv + argc, "--input"))
		return false;
	return true;
}

VideoStitcher::VideoStitcher(int argc, char* argv[]) {
	mVL = new VideoLoader( getCmdOption(argv, argv + argc, "--input") );
	mRPG = new RefProjGenerator(mVL);
	mRPG->doReferenceProjection();
}

int main(int argc, char* argv[]) {
	if ( !checkArguments(argc, argv) )
		exitWithMsg(E_BAD_ARGUMENTS);
	VideoStitcher* vs = new VideoStitcher(argc, argv);
}