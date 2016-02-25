#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <header/VideoStitch.h>

void segFaultHandler (int sig) {
  void *array[10];
  size_t size;

  size = backtrace(array, 10);

  logMsg(LOG_ERROR, stringFormat("Error: signal %d:\n", sig));
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}


int main(int argc, char *argv[]) {
	signal(SIGSEGV, segFaultHandler);

	std::unique_ptr<VideoStitcher> vs ( new VideoStitcher(argc, argv) );
    vs->doRealTimeStitching(argc, argv);
    vs.release();
}
