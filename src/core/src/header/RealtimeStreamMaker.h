#ifndef _H_REAL_TIME_STREAM_MAKER
#define _H_REAL_TIME_STREAM_MAKER

#include <thread>
#include <queue>

#include <header/Params.h>
#include <header/Usage.h>

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <glib.h>

#include "opencv2/core/core.hpp"
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;

typedef struct {
  gboolean white;
  GstClockTime timestamp;
} MyContext;

class RealtimeStreamMaker {
	private:
		static queue<Mat> frameQueue;
		static Mat latestFrame;
		static GMainLoop* loop;
		static GstRTSPServer *server;
		static GstRTSPMountPoints  *mounts;
		static GstRTSPMediaFactory* factory;
		thread* serverThread;
		
	public:
		static void need_data(GstElement *appsrc, guint unused, MyContext *ctx);
		static void media_configure(GstRTSPMediaFactory *factory, GstRTSPMedia *media, gpointer user_data);
		static void runInBackground();
		void waitForServerFinish();
		void streamOutFrame(Mat frame);
    	RealtimeStreamMaker(int argc, char* argv[]);
    	~RealtimeStreamMaker();
};

#endif // _H_REAL_TIME_STREAM_MAKER
