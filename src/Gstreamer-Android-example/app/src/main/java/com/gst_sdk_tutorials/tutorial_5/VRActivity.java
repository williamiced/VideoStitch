package com.gst_sdk_tutorials.tutorial_5;

import android.app.ActionBar;
import android.content.Context;
import android.graphics.SurfaceTexture;
import android.os.Build.VERSION;
import android.os.Build.VERSION_CODES;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Toast;

import com.google.vrtoolkit.cardboard.CardboardActivity;
import com.google.vrtoolkit.cardboard.CardboardView;

import org.freedesktop.gstreamer.GStreamer;
import org.rajawali3d.materials.textures.StreamingTexture;

/**
 * @author dennis.ippel
 */
public class VRActivity extends CardboardActivity {
    private native void nativeInit(boolean isTCP);     // Initialize native code, build pipeline, etc
    private native void nativeFinalize(); // Destroy pipeline and shutdown native code
    private native void nativePlay();     // Set pipeline to PLAYING
    private native void nativePause();    // Set pipeline to PAUSED
    private static native boolean nativeClassInit(); // Initialize native class: cache Method IDs for callbacks
    private native void nativeSurfaceInit(Object surface);
    private native void nativeSurfaceFinalize();
    private long native_custom_data;

    private Context mContext;
    private CardboardView mSurfaceView;
    private MyRenderer mRenderer;

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
        mSurfaceView = new VRSurfaceView(this);
        mRenderer = new MyRenderer(this, new StreamingTexture.ISurfaceListener() {
            @Override
            public void setSurface(Surface surface) {
                Log.d("MyVRActivity", "Run surface init: " + surface);
                nativeSurfaceInit(surface);
            }
        });
        setRenderer(mRenderer);

        addContentView(mSurfaceView, new ActionBar.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT));

        int uiFlags = View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION // hide nav bar
                | View.SYSTEM_UI_FLAG_FULLSCREEN; // hide status bar
        if (VERSION.SDK_INT >= VERSION_CODES.KITKAT) {
            uiFlags |= View.SYSTEM_UI_FLAG_IMMERSIVE;
        }

        mSurfaceView.setSystemUiVisibility(uiFlags);

        setCardboardView(mSurfaceView);

        nativeInit(true);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mContext = this;

        initGStreamer();
        initCardboard();
    }

    protected void setRenderer(VRRenderer renderer) {
        mSurfaceView.setRenderer(renderer);
    }

    public CardboardView getSurfaceView() {
        return mSurfaceView;
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

    @Override
    public void onResume() {
        super.onResume();
    }

    @Override
    public void onPause() {
        super.onPause();
    }
}