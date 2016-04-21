#ifndef _H_REAL_TIME_STREAM_MAKER
#define _H_REAL_TIME_STREAM_MAKER

#include <thread>
#include <queue>
#include <string>

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

typedef struct _App {
  GstElement* pipeline;
  GstElement* appsrc;
  GstElement* appsrc_small;
  GMainLoop* loop;
  guint sourceid;
  guint sourceid_small;
  GstClockTime timestamp;
  GstClockTime timestamp_small;
  GTimer* timer;
  GTimer* timer_small;
} MyApp;

class RealtimeStreamMaker {
	private:
		static queue<Mat> frameQueue;
		static queue<Mat> frameQueue_small;
		static Mat latestFrame;
		static Mat latestFrame_small;
		static MyApp* mApp;
		thread* serverThread;
		
	public:
		static gboolean read_data(MyApp* app);
		static void start_feed (GstElement* pl, guint size, MyApp* app);
		static void stop_feed (GstElement* pl, MyApp* app);
		static gboolean read_data_small(MyApp* app);
		static void start_feed_small (GstElement* pl, guint size, MyApp* app);
		static void stop_feed_small (GstElement* pl, MyApp* app);
		static void runInBackground();
		void waitForServerFinish();
		void streamOutFrame(Mat frame);
		void streamOutFrame_small(Mat frame);
    	RealtimeStreamMaker(int argc, char* argv[], string clientIP);
    	~RealtimeStreamMaker();
};

#endif // _H_REAL_TIME_STREAM_MAKER
