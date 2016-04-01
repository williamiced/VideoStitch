#include <header/RealtimeStreamMaker.h>

GMainLoop* RealtimeStreamMaker::loop = nullptr;
GstRTSPServer* RealtimeStreamMaker::server = nullptr;
GstRTSPMountPoints* RealtimeStreamMaker::mounts = nullptr;
GstRTSPMediaFactory* RealtimeStreamMaker::factory = nullptr;
queue<Mat> RealtimeStreamMaker::frameQueue;
Mat RealtimeStreamMaker::latestFrame = Mat::zeros(OUTPUT_PANO_HEIGHT, OUTPUT_PANO_WIDTH, CV_8UC3);;

void RealtimeStreamMaker::streamOutFrame(Mat frame) {
	frameQueue.push(frame);
    //logMsg(LOG_DEBUG, stringFormat("Frame queue now contains %d frames", frameQueue.size() ) );
}

void RealtimeStreamMaker::need_data(GstElement *appsrc, guint unused, MyContext *ctx) {
    GstBuffer *buffer;
    guint size;
    GstFlowReturn ret;

    GstMapInfo info;
    //Mat latestFrame = imread("dog.jpg");
    //cvtColor(latestFrame, latestFrame, CV_BGR2RGB);
    //size = latestFrame.cols * latestFrame.rows * latestFrame.channels();

    //static bool isFirstTime = true;
    //if (!isFirstTime)
    //    return;
    
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

	/* Increment the timestamp for 25 fps */
    GST_BUFFER_PTS(buffer)      = ctx->timestamp;
    GST_BUFFER_DURATION(buffer) = gst_util_uint64_scale_int(1, GST_SECOND, 40);
    ctx->timestamp             += GST_BUFFER_DURATION(buffer);

    g_signal_emit_by_name(appsrc, "push-buffer", buffer, &ret);

    gst_buffer_unref(buffer);
}

void RealtimeStreamMaker::media_configure(GstRTSPMediaFactory *factory, GstRTSPMedia *media, gpointer user_data) {
    GstElement *element, *appsrc;
    MyContext  *ctx;

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
                                 "width", G_TYPE_INT, OUTPUT_PANO_WIDTH,
                                 "height", G_TYPE_INT, OUTPUT_PANO_HEIGHT,
                                 "framerate", GST_TYPE_FRACTION, 25, 1, NULL), NULL);

    ctx = g_new0(MyContext, 1);
    ctx->timestamp = 0;

    /* make sure the data is freed when the media is gone */
    g_object_set_data_full(G_OBJECT(media), "my-extra-data", ctx, (GDestroyNotify)g_free);

    /* install the callback that will be called when a buffer is needed */
    g_signal_connect(appsrc, "need-data", (GCallback)need_data, ctx);
    gst_object_unref(appsrc);
    gst_object_unref(element);
}

void RealtimeStreamMaker::runInBackground() {
	g_print("stream ready at rtsp://127.0.0.1:8554/test\n");
    g_main_loop_run(loop);
}

void RealtimeStreamMaker::waitForServerFinish() {
	serverThread->join();
}

RealtimeStreamMaker::RealtimeStreamMaker(int argc, char* argv[]) {
	gst_init(&argc, &argv);

	loop   = g_main_loop_new(NULL, FALSE);
    server = gst_rtsp_server_new();
    mounts  = gst_rtsp_server_get_mount_points(server);
    factory = gst_rtsp_media_factory_new();

    // Set pipeline for app
    gst_rtsp_media_factory_set_launch(factory,
                                  "( appsrc name=mysrc ! videoconvert ! capsfilter ! x264enc speed-preset=ultrafast tune=zerolatency ! rtph264pay name=pay0 pt=96 )");
    gst_rtsp_media_factory_set_shared(GST_RTSP_MEDIA_FACTORY(factory), TRUE);

    g_signal_connect(factory, "media-configure", (GCallback)media_configure, NULL);
    gst_rtsp_mount_points_add_factory(mounts, "/test", factory);

    /* don't need the ref to the mounts anymore */
    g_object_unref(mounts);

    /* attach the server to the default maincontext */
    gst_rtsp_server_attach(server, NULL);

    serverThread = new thread(runInBackground);
}

RealtimeStreamMaker::~RealtimeStreamMaker() {

}