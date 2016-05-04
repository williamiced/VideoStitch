package com.gst_sdk_tutorials.tutorial_5;

import java.io.DataOutputStream;
import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.Socket;

import android.app.Activity;
import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Environment;
import android.util.Log;

public class MySensorClient implements SensorEventListener {
    DatagramSocket mSocket;
    String mDstAddress;
    volatile String mMsg;
    boolean mIsKilled;
    int mDstPort;
    Thread mThread;
    Object mMutex;

    private Activity mActivity;

    private SensorEventListener mSEL;
    private SensorManager mSensorManager;
    private Sensor mAccelerometer;
    private Sensor mMagnetometer;
    private float[] mLastAccelerometer = new float[3];
    private float[] mLastMagnetometer = new float[3];
    private boolean mLastAccelerometerSet = false;
    private boolean mLastMagnetometerSet = false;
    private float[] mR = new float[9];
    private float[] mOrientation = new float[3];

    MySensorClient(Activity context, String addr, int port) {
        mActivity = context;
        mDstAddress = addr;
        mDstPort = port;
        mSocket = null;
        mIsKilled = false;
        mMutex = new Object();
        mMsg = new String("");
        initForSensors();
        constructConnection();
    }

    public boolean isExternalStorageWritable() {
        String state = Environment.getExternalStorageState();
        if (Environment.MEDIA_MOUNTED.equals(state)) {
            return true;
        }
        return false;
    }

    private void initForSensors() {
        mSEL = this;

        if (!isExternalStorageWritable()) {
            Log.i("HeadMovementPredictor", "Cannot write files");
            mActivity.finish();
        }

        mSensorManager = (SensorManager) mActivity.getSystemService(Context.SENSOR_SERVICE);
        mAccelerometer = mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
        mMagnetometer = mSensorManager.getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD);
        mLastAccelerometerSet = false;
        mLastMagnetometerSet = false;
        mSensorManager.registerListener(mSEL, mAccelerometer, SensorManager.SENSOR_DELAY_FASTEST);
        mSensorManager.registerListener(mSEL, mMagnetometer, SensorManager.SENSOR_DELAY_FASTEST);
    }

    private void constructConnection() {
        try {
            mThread = new Thread(new Runnable() {
                @Override
                public void run() {
                    try {
                        mSocket = new DatagramSocket(mDstPort);

                        while ( !mIsKilled ) {
                            boolean isEmpty = false;
                            synchronized (mMutex) {
                                isEmpty = mMsg.isEmpty();
                            }
                            if (isEmpty)
                                Thread.sleep(1); // Sleep
                            else {
                                DatagramPacket packet;
                                synchronized (mMutex) {
                                    packet = new DatagramPacket(mMsg.getBytes(), mMsg.length(), InetAddress.getByName(mDstAddress), mDstPort);
                                    mMsg = "";
                                }
                                mSocket.send(packet);
                            }
                        }
                    } catch (Exception ex) {
                        ex.printStackTrace();
                        Log.e("MyClient", "Construction of connection failed");
                    }
                }
            });
            mThread.start();
        } catch (Exception ex) {
            ex.printStackTrace();
        }
    }

    public void waitThread() {
        mThread.interrupt();
        try {
            mThread.join();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    public void sendMsg(String msg) {
        synchronized (mMutex) {
            mMsg = msg;
        }
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
        if (mLastAccelerometerSet && mLastMagnetometerSet) { // Update synchronized with lower fps sensor
            SensorManager.getRotationMatrix(mR, null, mLastAccelerometer, mLastMagnetometer);
            SensorManager.getOrientation(mR, mOrientation);
            sendMsg(String.format("%f,%f,%f", mOrientation[0], mOrientation[1], mOrientation[2]));
            //Log.d("MyVR", String.format("%f,%f,%f", mOrientation[0], mOrientation[1], mOrientation[2]));
        }
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {
        // Do nothing
    }
}