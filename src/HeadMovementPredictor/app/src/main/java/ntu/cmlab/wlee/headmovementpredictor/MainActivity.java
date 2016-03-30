package ntu.cmlab.wlee.headmovementpredictor;

import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.TextView;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;

public class MainActivity extends AppCompatActivity implements SensorEventListener {
    private SensorManager mSensorManager;
    private Sensor mAccelerometer;
    private Sensor mMagnetometer;

    private float[] mLastAccelerometer = new float[3];
    private float[] mLastMagnetometer = new float[3];
    private boolean mLastAccelerometerSet = false;
    private boolean mLastMagnetometerSet = false;

    private float[] mR = new float[9];
    private float[] mOrientation = new float[3];

    private Handler mHandler = new Handler();
    private SensorEventListener mSEL;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mSEL = this;

        if (!isExternalStorageWritable()) {
            Log.i("HeadMovementPredictor", "Cannot write files");
            finish();
        }

        // Give time to put the cell phone into the cardboard
        Thread thr = new Thread(new Runnable() {
            @Override
            public void run() {
                for (int i=10; i>0; i--) {
                    final int finalI = i;
                    mHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            Log.i("abc", new String(finalI + ""));
                            ((TextView) findViewById(R.id.timeText)).setText(new String(finalI + ""));
                        }
                    });
                    try {
                        Thread.sleep(1000);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }

                mHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        truncateRecordFile();
                        ((TextView)findViewById(R.id.timeText)).setVisibility(View.INVISIBLE);
                        ((TextView)findViewById(R.id.hintText)).setText("Start to record");

                        mSensorManager = (SensorManager)getSystemService(SENSOR_SERVICE);
                        mAccelerometer = mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
                        mMagnetometer = mSensorManager.getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD);

                        mLastAccelerometerSet = false;
                        mLastMagnetometerSet = false;
                        mSensorManager.registerListener(mSEL, mAccelerometer, SensorManager.SENSOR_DELAY_NORMAL);
                        mSensorManager.registerListener(mSEL, mMagnetometer, SensorManager.SENSOR_DELAY_NORMAL);
                    }
                });
            }
        });
        thr.start();
    }

    @Override
    protected void onResume() {
        super.onResume();
        mLastAccelerometerSet = false;
        mLastMagnetometerSet = false;
        if (mSensorManager != null) {
            mSensorManager.registerListener(this, mAccelerometer, SensorManager.SENSOR_DELAY_NORMAL);
            mSensorManager.registerListener(this, mMagnetometer, SensorManager.SENSOR_DELAY_NORMAL);
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        if (mSensorManager != null)
            mSensorManager.unregisterListener(this);
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
            appendLog(String.format("%f,%f,%f,%f,%f", mOrientation[0], mOrientation[1], mOrientation[2], mLastAccelerometer[0], mLastAccelerometer[1], mLastAccelerometer[2]));
        }
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {

    }

    private void appendLog(String text) {
        File logFile = new File(Environment.getExternalStoragePublicDirectory(
                Environment.DIRECTORY_DOCUMENTS), "Record.txt");
        if (!logFile.exists()) {
            try {
                logFile.createNewFile();
            } catch (IOException e) {
                e.printStackTrace();
                finish();
            }
        }
        BufferedWriter buf;
        try {
            //BufferedWriter for performance, true to set append to file flag
            buf = new BufferedWriter(new FileWriter(logFile, true));
            buf.append(text);
            buf.newLine();
            buf.close();
        } catch (IOException e) {
            e.printStackTrace();
            finish();
        }
    }

    /* Checks if external storage is available for read and write */
    public boolean isExternalStorageWritable() {
        String state = Environment.getExternalStorageState();
        if (Environment.MEDIA_MOUNTED.equals(state)) {
            return true;
        }
        return false;
    }

    private void truncateRecordFile() {
        File logFile = new File(Environment.getExternalStoragePublicDirectory(
                Environment.DIRECTORY_DOCUMENTS), "Record.txt");
        logFile.delete();
    }
}
