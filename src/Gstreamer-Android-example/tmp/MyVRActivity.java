package com.gst_sdk_tutorials.tutorial_5;

import com.google.vrtoolkit.cardboard.CardboardActivity;

import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import android.view.Surface;
import android.widget.Toast;

import org.freedesktop.gstreamer.GStreamer;
import org.rajawali3d.cardboard.RajawaliCardboardView;
import org.rajawali3d.materials.textures.StreamingTexture;

public class MyVRActivity extends CardboardActivity {
    private native void nativeInit(boolean isTCP);     // Initialize native code, build pipeline, etc
    private native void nativeFinalize(); // Destroy pipeline and shutdown native code
    private native void nativePlay();     // Set pipeline to PLAYING
    private native void nativePause();    // Set pipeline to PAUSED
    private static native boolean nativeClassInit(); // Initialize native class: cache Method IDs for callbacks
    private native void nativeSurfaceInit(Object surface);
    private native void nativeSurfaceFinalize();
    private long native_custom_data;
    private Surface mSurface;

    private Context mContext;

    MyRenderer mRenderer;
    RajawaliCardboardView mCardboardView;

    static {
        System.loadLibrary("gstreamer_android");
        System.loadLibrary("my_rtsp_client");
        nativeClassInit();
    }

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

    private void initCardboard() {
        mCardboardView = new RajawaliCardboardView(this);
        setContentView(mCardboardView);
        setCardboardView(mCardboardView);

        mRenderer = new MyRenderer(this, new StreamingTexture.ISurfaceListener() {
            @Override
            public void setSurface(Surface surface) {
                Log.d("MyVRActivity", "Run surface init: " + surface);
                mSurface = surface;
                nativeSurfaceInit(mSurface);
            }
        });

        mCardboardView.setRenderer(mRenderer);
        mCardboardView.setSurfaceRenderer(mRenderer);

        nativeInit(true);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mContext = this;
        initGStreamer();
        initCardboard();
    }

    @Override
    protected void onDestroy() {
        nativeFinalize();
        super.onDestroy();
    }

    private void onGStreamerInitialized () {
        nativePlay();
    }

    private void setMessage(final String message){
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Toast.makeText(mContext, message, Toast.LENGTH_LONG).show();
            }
        });
    }
}
