#ifndef _H_REAL_TIME_STREAM_MAKER
#define _H_REAL_TIME_STREAM_MAKER

#include <thread>
#include <queue>
#include <string>

#include <header/Params.h>
#include <header/Usage.h>

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
//#include <gst/rtsp-server/rtsp-server.h>
#include <glib.h>

#include "opencv2/core/core.hpp"
#include "opencv2/opencv.hpp"

using namespace std;

typedef struct _App {
  GstElement* pipeline;
  GstElement* appsrc;
  GMainLoop* loop;
  guint sourceid;
  GstClockTime timestamp;
  GTimer* timer;
  //GstRTSPServer* server;
  //GstRTSPMountPoints* mounts;
  //GstRTSPMediaFactory* factory;
} MyApp;

class RealtimeStreamMaker {
	private:
		static queue<cv::Mat> frameQueue;
		static cv::Mat latestFrame;
		static MyApp* mApp;
		thread* serverThread;
		
	public:
		static gboolean read_data(MyApp* app);
		static void start_feed (GstElement* pl, guint size, MyApp* app);
		static void stop_feed (GstElement* pl, MyApp* app);
		static void runInBackground();
		//static void media_configure(GstRTSPMediaFactory *factory, GstRTSPMedia *media, gpointer user_data);
		static void rtspNeedData (GstElement* pl, guint unused_size, MyApp* app);
		void waitForServerFinish();
		void streamOutFrame(cv::Mat frame);
    	RealtimeStreamMaker(int argc, char* argv[], string clientIP);
    	~RealtimeStreamMaker();
};

#endif // _H_REAL_TIME_STREAM_MAKER
