#include <header/VideoStitch.h>

int main(int argc, char *argv[]) {
	std::unique_ptr<VideoStitcher> vs ( new VideoStitcher(argc, argv) );
    vs->doRealTimeStitching(argc, argv);
    vs.release();
}
