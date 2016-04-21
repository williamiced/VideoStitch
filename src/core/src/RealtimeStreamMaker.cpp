#include <header/RealtimeStreamMaker.h>

queue<Mat> RealtimeStreamMaker::frameQueue;
Mat RealtimeStreamMaker::latestFrame = Mat::zeros(OUTPUT_PANO_HEIGHT, OUTPUT_PANO_WIDTH, CV_8UC3);;
MyApp* RealtimeStreamMaker::mApp;

void RealtimeStreamMaker::streamOutFrame(Mat frame) {
	frameQueue.push(frame);
}

gboolean RealtimeStreamMaker::read_data(MyApp* app) {
    GstBuffer *buffer;
    guint size;
    GstFlowReturn ret;
    GstMapInfo info;

    gdouble ms = g_timer_elapsed(app->timer, NULL);
    if (ms > 1.0/30.0) {
        if (frameQueue.size() > 0) {
            latestFrame = frameQueue.front();
            cvtColor(latestFrame, latestFrame, CV_BGR2RGB);
            frameQueue.pop();
        }

        size = latestFrame.cols * latestFrame.rows * latestFrame.channels();
        buffer = gst_buffer_new_allocate(NULL, size, NULL);     

        gst_buffer_map (buffer, &info, GST_MAP_WRITE);
        memcpy (info.data, latestFrame.data, size);
        gst_buffer_unmap (buffer, &info);

        // For 30 fps
        GST_BUFFER_PTS(buffer)      = app->timestamp;
        GST_BUFFER_DURATION(buffer) = gst_util_uint64_scale_int(1, GST_SECOND, 33);
        app->timestamp             += GST_BUFFER_DURATION(buffer);

        g_signal_emit_by_name(app->appsrc, "push-buffer", buffer, &ret);
        gst_buffer_unref(buffer);

        g_timer_start(app->timer);
    }
    return TRUE;
}

void RealtimeStreamMaker::start_feed (GstElement* pl, guint unused_size, MyApp* app) {
    if (app->sourceid == 0) {
        logMsg(LOG_DEBUG, "start feeding", 1);
        app->sourceid = g_idle_add ((GSourceFunc) read_data, app);
    }
}

void RealtimeStreamMaker::stop_feed (GstElement* pl, MyApp* app) {
    if (app->sourceid != 0) {
        logMsg(LOG_DEBUG, "stop feeding", 1);
        g_source_remove (app->sourceid);
        app->sourceid = 0;
    }
}

void RealtimeStreamMaker::runInBackground() {
    logMsg(LOG_INFO, "stream ready at rtsp://127.0.0.1:8554/test", 1);
    gst_element_set_state (mApp->pipeline, GST_STATE_PLAYING);
    g_main_loop_run(mApp->loop);
}

void RealtimeStreamMaker::waitForServerFinish() {
	serverThread->join();

    /* clean up */
    gst_element_set_state (mApp->pipeline, GST_STATE_NULL);
    gst_object_unref (GST_OBJECT (mApp->pipeline));
    g_main_loop_unref (mApp->loop);
}

RealtimeStreamMaker::RealtimeStreamMaker(int argc, char* argv[], string clientIP) {
	gst_init(&argc, &argv);

    mApp = g_new0(MyApp, 1);
	mApp->timestamp = 0;
    mApp->loop   = g_main_loop_new(NULL, FALSE);
    mApp->timer = g_timer_new();
#ifdef USING_UDP
    mApp->pipeline = gst_parse_launch(
        stringFormat("appsrc name=mysrc format=time ! videoconvert ! x264enc tune=zerolatency sliced-threads=true ! rtph264pay ! udpsink host=%s port=5000 ", clientIP.c_str()).c_str(), NULL);
#else
    mApp->pipeline = gst_parse_launch(
        stringFormat("appsrc name=mysrc format=time ! videoconvert ! x264enc tune=zerolatency ! tcpserversink host=0.0.0.0 port=5000").c_str(), NULL);
#endif
    g_assert (mApp->pipeline);

    mApp->appsrc = gst_bin_get_by_name (GST_BIN(mApp->pipeline), "mysrc");
    g_signal_connect (mApp->appsrc, "need-data", G_CALLBACK (start_feed), mApp);
    g_signal_connect (mApp->appsrc, "enough-data", G_CALLBACK (stop_feed), mApp);

    GstCaps* caps = gst_caps_new_simple ("video/x-raw",
                     "format", G_TYPE_STRING, "RGB",
                     "width", G_TYPE_INT, OUTPUT_PANO_WIDTH,
                     "height", G_TYPE_INT, OUTPUT_PANO_HEIGHT,
                     "framerate", GST_TYPE_FRACTION, 30, 1,
                     NULL);
    g_object_set (G_OBJECT (mApp->appsrc), "caps", caps, NULL);
    gst_element_set_state (mApp->pipeline, GST_STATE_PLAYING);

    serverThread = new thread(runInBackground);
    //runInBackground();
}

RealtimeStreamMaker::~RealtimeStreamMaker() {
}