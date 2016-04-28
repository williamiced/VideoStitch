package com.gst_sdk_tutorials.tutorial_5;

import java.io.DataOutputStream;
import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.Socket;
import android.util.Log;

public class MyClient {
    DatagramSocket mSocket;
    String mDstAddress;
    volatile String mMsg;
    boolean mIsKilled;
    int mDstPort;
    Thread mThread;
    Object mMutex;

    MyClient(String addr, int port) {
        mDstAddress = addr;
        mDstPort = port;
        mSocket = null;
        mIsKilled = false;
        mMutex = new Object();
        mMsg = new String("");
        constructConnection();
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
                                Thread.sleep(10); // Sleep
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

    private void waitThread() {
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
}