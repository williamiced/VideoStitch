#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <header/VideoStitch.h>

int main(int argc, char *argv[]) {
	signal(SIGSEGV, segFaultHandler);

	std::unique_ptr<VideoStitcher> vs ( new VideoStitcher(argc, argv) );
    vs->doRealTimeStitching(argc, argv);
    vs.release();
}
