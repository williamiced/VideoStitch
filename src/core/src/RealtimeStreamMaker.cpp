#include <header/RealtimeStreamMaker.h>

queue<Mat> RealtimeStreamMaker::frameQueue;
Mat RealtimeStreamMaker::latestFrame;
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
        //logMsg(LOG_DEBUG, "start feeding", 1);
        app->sourceid = g_idle_add ((GSourceFunc) read_data, app);
    }
}

void RealtimeStreamMaker::stop_feed (GstElement* pl, MyApp* app) {
    if (app->sourceid != 0) {
        //logMsg(LOG_DEBUG, "stop feeding", 1);
        g_source_remove (app->sourceid);
        app->sourceid = 0;
    }
}

void RealtimeStreamMaker::rtspNeedData (GstElement* pl, guint unused_size, MyApp* app) {
    GstBuffer *buffer;
    guint size;
    GstFlowReturn ret;
    GstMapInfo info;

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

    g_signal_emit_by_name(pl, "push-buffer", buffer, &ret);
    gst_buffer_unref(buffer);
}

void RealtimeStreamMaker::runInBackground() {
    logMsg(LOG_INFO, "stream ready at 140.112.29.188", 1);
    g_main_loop_run(mApp->loop);
}

void RealtimeStreamMaker::waitForServerFinish() {
	serverThread->join();

    /* clean up */
    gst_element_set_state (mApp->pipeline, GST_STATE_NULL);
    gst_object_unref (GST_OBJECT (mApp->pipeline));
    g_main_loop_unref (mApp->loop);
}

void RealtimeStreamMaker::media_configure(GstRTSPMediaFactory *factory, GstRTSPMedia *media, gpointer user_data) {
    logMsg(LOG_INFO, "Media configured", 1);

    GstElement *element, *appsrc;

    /* get the element used for providing the streams of the media */
    element = gst_rtsp_media_get_element(media);

    /* get our appsrc, we named it 'mysrc' with the name property */
    appsrc = gst_bin_get_by_name_recurse_up(GST_BIN(element), "mysrc");

    /* this instructs appsrc that we will be dealing with timed buffer */
    gst_util_set_object_arg(G_OBJECT(appsrc), "format", "time");

    /* configure the caps of the video */
    g_object_set(G_OBJECT(appsrc), "caps",
             gst_caps_new_simple("video/x-raw",
                                 "format", G_TYPE_STRING, "RGB",
                                 "width", G_TYPE_INT, getIntConfig("OUTPUT_PANO_WIDTH"),
                                 "height", G_TYPE_INT, getIntConfig("OUTPUT_PANO_HEIGHT"),
                                 "framerate", GST_TYPE_FRACTION, 30, 1, 
                                 NULL), NULL);

    /* make sure the data is freed when the media is gone */
    //g_object_set_data_full(G_OBJECT(media), "my-extra-data", ctx, (GDestroyNotify)g_free);

    /* install the callback that will be called when a buffer is needed */
    g_signal_connect (appsrc, "need-data", G_CALLBACK (rtspNeedData), mApp);
    gst_object_unref(appsrc);
    gst_object_unref(element);
}

RealtimeStreamMaker::RealtimeStreamMaker(int argc, char* argv[], string clientIP) {
	gst_init(&argc, &argv);

    latestFrame = Mat::zeros(getIntConfig("OUTPUT_PANO_HEIGHT"), getIntConfig("OUTPUT_PANO_WIDTH"), CV_8UC3);

    mApp = g_new0(MyApp, 1);
	mApp->timestamp = 0;
    mApp->loop   = g_main_loop_new(NULL, FALSE);
    mApp->timer = g_timer_new();

    if (getStringConfig("USING_PROTOCAL").compare("RTSP") == 0 ) {
        mApp->server = gst_rtsp_server_new();
        mApp->mounts  = gst_rtsp_server_get_mount_points(mApp->server);
        mApp->factory = gst_rtsp_media_factory_new();

        // Set pipeline for app
        /*
        gst_rtsp_media_factory_set_launch(mApp->factory,
                                      "( appsrc name=mysrc ! videoconvert ! capsfilter ! x264enc tune=zerolatency ! rtph264pay name=pay0 pt=96 )");
                                      */
        gst_rtsp_media_factory_set_launch(mApp->factory,
                                      "( appsrc name=mysrc ! videoconvert ! capsfilter ! x264enc tune=zerolatency ! rtph264pay name=pay0 pt=96 )");
        gst_rtsp_media_factory_set_shared(GST_RTSP_MEDIA_FACTORY(mApp->factory), TRUE);

        g_signal_connect(mApp->factory, "media-configure", G_CALLBACK (media_configure), NULL);
        gst_rtsp_mount_points_add_factory(mApp->mounts, "/test", mApp->factory);

        /* don't need the ref to the mounts anymore */
        g_object_unref(mApp->mounts);

        /* attach the server to the default maincontext */
        gst_rtsp_server_attach(mApp->server, NULL);
    } else {
        if (getStringConfig("USING_PROTOCAL").compare("UDP") == 0 ) {
            mApp->pipeline = gst_parse_launch(
                stringFormat("appsrc name=mysrc format=time ! videoconvert ! x264enc tune=zerolatency sliced-threads=true ! rtph264pay ! udpsink host=%s port=5000 ", clientIP.c_str()).c_str(), NULL);
        } else if (getStringConfig("USING_PROTOCAL").compare("TCP") == 0 ) {
            mApp->pipeline = gst_parse_launch(
                stringFormat("appsrc name=mysrc format=time ! videoconvert ! x264enc tune=zerolatency ! tcpserversink host=0.0.0.0 port=5000").c_str(), NULL);
        } 

        g_assert (mApp->pipeline);

        mApp->appsrc = gst_bin_get_by_name (GST_BIN(mApp->pipeline), "mysrc");
        g_signal_connect (mApp->appsrc, "need-data", G_CALLBACK (start_feed), mApp);
        g_signal_connect (mApp->appsrc, "enough-data", G_CALLBACK (stop_feed), mApp);

        GstCaps* caps = gst_caps_new_simple ("video/x-raw",
                         "format", G_TYPE_STRING, "RGB",
                         "width", G_TYPE_INT, getIntConfig("OUTPUT_PANO_WIDTH"),
                         "height", G_TYPE_INT, getIntConfig("OUTPUT_PANO_HEIGHT"),
                         "framerate", GST_TYPE_FRACTION, 30, 1,
                         NULL);
        g_object_set (G_OBJECT (mApp->appsrc), "caps", caps, NULL);

        gst_element_set_state (mApp->pipeline, GST_STATE_PLAYING);
    }

    serverThread = new thread(runInBackground);
}

RealtimeStreamMaker::~RealtimeStreamMaker() {
}