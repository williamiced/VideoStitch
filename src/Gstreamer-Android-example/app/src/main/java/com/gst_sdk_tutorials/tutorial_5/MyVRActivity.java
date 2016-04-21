package com.gst_sdk_tutorials.tutorial_5;

import com.google.vrtoolkit.cardboard.CardboardActivity;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.graphics.Bitmap;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import org.freedesktop.gstreamer.GStreamer;
import org.rajawali3d.cardboard.RajawaliCardboardView;

public class MyVRActivity extends CardboardActivity implements SensorEventListener {
    private native void nativeInit(boolean isTCP);     // Initialize native code, build pipeline, etc
    private native void nativeFinalize(); // Destroy pipeline and shutdown native code
    private native void nativePlay();     // Set pipeline to PLAYING
    private native void nativePause();    // Set pipeline to PAUSED
    private static native boolean nativeClassInit(); // Initialize native class: cache Method IDs for callbacks
    private long native_custom_data;

    private SensorManager mSensorManager;
    private Sensor mAccelerometer;
    private Sensor mMagnetometer;

    private float[] mLastAccelerometer = new float[3];
    private float[] mLastMagnetometer = new float[3];
    private boolean mLastAccelerometerSet = false;
    private boolean mLastMagnetometerSet = false;

    private float[] mR = new float[9];
    private float[] mOrientation = new float[3];

    private SensorEventListener mSEL;
    private Context mContext;

    private boolean mIsPlaying = false;
    private boolean mIsInitialized = false;

    MyRenderer mRenderer;

    MyClient mSensorClient;

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

    private void initForSensors() {
        mSEL = this;

        if (!isExternalStorageWritable()) {
            Log.i("HeadMovementPredictor", "Cannot write files");
            finish();
        }

        mSensorManager = (SensorManager)getSystemService(SENSOR_SERVICE);
        mAccelerometer = mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
        mMagnetometer = mSensorManager.getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD);
        mLastAccelerometerSet = false;
        mLastMagnetometerSet = false;
        mSensorManager.registerListener(mSEL, mAccelerometer, SensorManager.SENSOR_DELAY_NORMAL);
        mSensorManager.registerListener(mSEL, mMagnetometer, SensorManager.SENSOR_DELAY_NORMAL);
    }

    private void initCardboard() {
        RajawaliCardboardView view = new RajawaliCardboardView(this);
        setContentView(view);
        setCardboardView(view);

        view.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                if (!mIsPlaying) {
                    if (!mIsInitialized) {
                        new AlertDialog.Builder(mContext)
                                .setTitle("Connect to server")
                                .setMessage("Which way? ")
                                .setPositiveButton("TCP", new DialogInterface.OnClickListener() {
                                    public void onClick(DialogInterface dialog, int which) {
                                        nativeInit(true);
                                        initSensorClient();
                                        mIsInitialized = true;
                                        nativePlay();
                                        mIsPlaying = true;
                                    }
                                })
                                .setNegativeButton("UDP", new DialogInterface.OnClickListener() {
                                    public void onClick(DialogInterface dialog, int which) {
                                        nativeInit(false);
                                        initSensorClient();
                                        mIsInitialized = true;
                                        nativePlay();
                                        mIsPlaying = true;
                                    }
                                })
                                .setIcon(android.R.drawable.ic_dialog_alert)
                                .show();
                    } else {
                        nativePlay();
                        mIsPlaying = true;
                    }
                } else {
                    nativePause();
                    mIsPlaying = false;
                }
                return false;
            }
        });

        mRenderer = new MyRenderer(this);
        view.setRenderer(mRenderer);
        view.setSurfaceRenderer(mRenderer);
    }

    private void initSensorClient() {
        mSensorClient = new MyClient(getString(R.string.addr), Integer.parseInt(getString(R.string.port)));
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mContext = this;
        initForSensors();
        initCardboard();
        initGStreamer();
    }

    @Override
    protected void onDestroy() {
        nativeFinalize();
        super.onDestroy();
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

    public boolean isExternalStorageWritable() {
        String state = Environment.getExternalStorageState();
        if (Environment.MEDIA_MOUNTED.equals(state)) {
            return true;
        }
        return false;
    }

    @Override
    public void onSensorChanged(SensorEvent event) {
        if (event.sensor == mAccelerometer) {
            System.arraycopy(event.values, 0, mLastAccelerometer, 0, event.values.length);
            mLastAccelerometerSet = true;
        } else if (event.sensor == mMagnetometer) {
            System.arraycopy(event.values, 0, mLastMagnetometer, 0, event.values.length);
            mLastMagnetometerSet = true;
        }
        if (mLastAccelerometerSet && mLastMagnetometerSet) {
            SensorManager.getRotationMatrix(mR, null, mLastAccelerometer, mLastMagnetometer);
            SensorManager.getOrientation(mR, mOrientation);
            if (mSensorClient != null && mIsPlaying) {
                mSensorClient.sendMsg(String.format("%f,%f,%f", mOrientation[0], mOrientation[1], mOrientation[2]));
            } else {
                //Log.d("MyVR", String.format("%f,%f,%f", mOrientation[0], mOrientation[1], mOrientation[2]));
            }
        }
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {

    }

    private void onGStreamerInitialized () {
        nativePause();
    }

    private void setMessage(final String message){
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Toast.makeText(mContext, message, Toast.LENGTH_LONG).show();
            }
        });
    }

    private void passData(final int width, final int height, final byte[] data) {
        final int oriLen = data.length;
        final int[] rgbArr = convertYUV444toRGB8888(data, width, height);
        final int len = rgbArr.length;
        /*
        final Bitmap bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
        bitmap.setPixels(rgbArr, 0, width, 0, 0, width, height);
        mRenderer.changeTextureByBitmap(bitmap);
        */
    }

    private void passDataSmall(final int width, final int height, final byte[] data) {
        final int oriLen = data.length;
        final int[] rgbArr = convertYUV444toRGB8888(data, width, height);
        final int len = rgbArr.length;

        //Log.d("MyVRActivity", "Get small!!!");

        final Bitmap bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
        bitmap.setPixels(rgbArr, 0, width, 0, 0, width, height);
        mRenderer.changeTextureByBitmap(bitmap);
    }
}
