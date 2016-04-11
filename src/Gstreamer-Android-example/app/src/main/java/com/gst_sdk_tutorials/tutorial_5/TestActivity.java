package com.gst_sdk_tutorials.tutorial_5;

import android.content.Context;
import android.graphics.Bitmap;
import android.media.Image;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import org.freedesktop.gstreamer.GStreamer;

public class TestActivity extends Activity {
    private native void nativeInit();     // Initialize native code, build pipeline, etc
    private native void nativeFinalize(); // Destroy pipeline and shutdown native code
    private native void nativePlay();     // Set pipeline to PLAYING
    private native void nativePause();    // Set pipeline to PAUSED
    private static native boolean nativeClassInit(); // Initialize native class: cache Method IDs for callbacks
    private long native_custom_data;      // Native code will use this to keep private data
    private boolean is_playing_desired;   // Whether the user asked to go to PLAYING
    private Context mContext;

    private void initGStreamer() {
        // Initialize GStreamer and warn if it fails
        try {
            GStreamer.init(this);
        } catch (Exception e) {
            Toast.makeText(this, e.getMessage(), Toast.LENGTH_LONG).show();
            finish();
            return;
        }
    }

    private void setBtnListner() {
        findViewById(R.id.playBtn).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                is_playing_desired = true;
                nativePlay();
            }
        });
        findViewById(R.id.pauseBtn).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                is_playing_desired = false;
                nativePause();
            }
        });
    }

    // Called when the activity is first created.
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_test);

        mContext = this;

        initGStreamer();
        setBtnListner();

        if (savedInstanceState != null) {
            is_playing_desired = savedInstanceState.getBoolean("playing");
            Log.i("GStreamer", "Activity created. Saved state is playing:" + is_playing_desired);
        } else {
            is_playing_desired = false;
            Log.i ("GStreamer", "Activity created. There is no saved state, playing: false");
        }

        nativeInit();
    }

    @Override
    protected void onDestroy() {
        nativeFinalize();
        super.onDestroy();
    }

    protected void onSaveInstanceState (Bundle outState) {
        Log.d("GStreamer", "Saving state, playing:" + is_playing_desired);
        outState.putBoolean("playing", is_playing_desired);
    }

    // Called from native code. Native code calls this once it has created its pipeline and
    // the main loop is running, so it is ready to accept commands.
    private void onGStreamerInitialized () {
        Log.i("GStreamer", "Gst initialized. Restoring state, playing:" + is_playing_desired);
        // Restore previous playing state
        if (is_playing_desired) {
            nativePlay();
        } else {
            nativePause();
        }

        // Re-enable buttons, now that GStreamer is initialized
        final Activity activity = this;
        runOnUiThread(new Runnable() {
            public void run() {
                activity.findViewById(R.id.playBtn).setEnabled(true);
                activity.findViewById(R.id.pauseBtn).setEnabled(true);
            }
        });
    }

    // Called from native code. This sets the content of the TextView from the UI thread.
    private void setMessage(final String message) {
        final TextView tv = (TextView) this.findViewById(R.id.textview_message);
        runOnUiThread(new Runnable() {
            public void run() {
                tv.setText(message);
            }
        });
    }

    private void testToast(final String message) {
        runOnUiThread (new Runnable() {
            public void run() {
                Toast.makeText(mContext, message, Toast.LENGTH_SHORT).show();
            }
        });
    }

    private void passData(final int width, final int height, final byte[] data) {
        final int oriLen = data.length;
        final int[] rgbArr = convertYUV444toRGB8888(data, width, height);
        final int len = rgbArr.length;
        final Bitmap bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
        bitmap.setPixels(rgbArr, 0, width, 0, 0, width, height);
        final int h = bitmap.getHeight();
        final ImageView im = (ImageView) findViewById(R.id.testImage);

        runOnUiThread(new Runnable() {
            public void run() {
                //Toast.makeText(mContext, "oriLen: "+oriLen+" RGB arr size: " + len + ", h=" + h, Toast.LENGTH_SHORT).show();
                im.setImageBitmap(bitmap);
            }
        });
    }

    static {
        System.loadLibrary("gstreamer_android");
        System.loadLibrary("my_rtsp_client");
        nativeClassInit();
    }

    public static int[] convertYUV444toRGB8888(byte [] data, int width, int height) {
        int size = width*height;
        int offset = size;
        int[] pixels = new int[size];
        int u, v, y;

        for(int i=0; i<width*height; i++) {
            y = data[i] & 0xff;
            u = data[width * height + i] & 0xff;
            v = data[2 * width * height + i] & 0xff;
            pixels[i] = convertYUVtoRGB(y, u, v);
        }

        return pixels;
    }

    private static int convertYUVtoRGB(int y, int u, int v) {
        int r,g,b;

        r = (int) (y + 1.13983*(v-128));
        g = (int) (y - 0.39465*(u-128)-0.58060*(v-128));
        b = (int) (y + 2.03211*(u-128));

        return 0xff000000 | (r<<16) | (g<<8) | b;
    }
}