package com.google.vrtoolkit.cardboard.samples.treasurehunt;

import android.opengl.GLES20;
import android.opengl.Matrix;

import com.google.vrtoolkit.cardboard.CardboardView;
import com.google.vrtoolkit.cardboard.Eye;
import com.google.vrtoolkit.cardboard.HeadTransform;
import com.google.vrtoolkit.cardboard.Viewport;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

import javax.microedition.khronos.egl.EGLConfig;

/**
 * Created by wlee on 2016/3/10.
 */
public class MyRenderer implements CardboardView.StereoRenderer {
    private float[] mCameraMat;
    private float[] mViewMat;
    private final float[] lightPosInEyeSpace = new float[4];

    private float[] mModelViewProjectionMat;
    private float[] mModelViewMat;
    private float[] mModelSphereMat;

    private int mSphereProgram;
    private int mSpherePositionParam;
    private int mSphereNormalParam;
    private int mSphereColorParam;
    private int mSphereModelParam;
    private int mSphereModelViewParam;
    private int mSphereModelViewProjectionParam;
    private int mSphereLightPosParam;

    private FloatBuffer mSphereVertices;
    private FloatBuffer mSphereColors;
    private FloatBuffer mSphereNormals;

    @Override
    public void onNewFrame(HeadTransform headTransform) {
        // Build the Model part of the ModelView matrix.
        Matrix.rotateM(mModelSphereMat, 0, Params.TIME_DELTA, 0.5f, 0.5f, 1.0f);

        // Build the camera matrix and apply it to the ModelView.
        Matrix.setLookAtM(mCameraMat, 0, 0.0f, 0.0f, Params.CAMERA_Z, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    }

    @Override
    public void onDrawEye(Eye eye) {
        GLES20.glEnable(GLES20.GL_DEPTH_TEST);
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT | GLES20.GL_DEPTH_BUFFER_BIT);

        // Apply eyes transformation to camera
        Matrix.multiplyMM(mViewMat, 0, eye.getEyeView(), 0, mCameraMat, 0);

        // Set the position of the light
        Matrix.multiplyMV(lightPosInEyeSpace, 0, mViewMat, 0, Params.LIGHT_POS_IN_WORLD_SPACE, 0);

        float[] perspective = eye.getPerspective(Params.Z_NEAR, Params.Z_FAR);
        Matrix.multiplyMM(mModelViewMat, 0, mViewMat, 0, mModelSphereMat, 0);
        Matrix.multiplyMM(mModelViewProjectionMat, 0, perspective, 0, mModelViewMat, 0);
        drawSphere();
    }

    @Override
    public void onFinishFrame(Viewport viewport) {

    }

    @Override
    public void onSurfaceChanged(int i, int i1) {
    }

    @Override
    public void onSurfaceCreated(EGLConfig eglConfig) {
        GLES20.glClearColor(0.0f, 0.0f, 0.0f, 0.5f);

        initBuffers();
        initSettings();
        setupSphereProgram();
    }

    @Override
    public void onRendererShutdown() {

    }

    private void initBuffers() {
        int mStep = 25;
        int mRaduis = 1;
        double DEG = Math.PI/180;

        double dTheta = mStep * DEG;
        double dPhi = dTheta;
        int points = 0;

        ByteBuffer bbVertices = ByteBuffer.allocateDirect(40000);
        bbVertices.order(ByteOrder.nativeOrder());
        mSphereVertices = bbVertices.asFloatBuffer();

        ByteBuffer bbColors = ByteBuffer.allocateDirect(40000);
        bbColors.order(ByteOrder.nativeOrder());
        mSphereColors = bbColors.asFloatBuffer();

        ByteBuffer bbNormals = ByteBuffer.allocateDirect(40000);
        bbNormals.order(ByteOrder.nativeOrder());
        mSphereNormals = bbColors.asFloatBuffer();


        for(double phi = -(Math.PI); phi <= Math.PI; phi+=dPhi) {
            //for each stage calculating the slices
            for(double theta = 0.0; theta <= (Math.PI * 2); theta+=dTheta) {
                mSphereVertices.put((float) (mRaduis * Math.sin(phi) * Math.cos(theta)) );
                mSphereVertices.put((float) (mRaduis * Math.sin(phi) * Math.sin(theta)) );
                mSphereVertices.put((float) (mRaduis * Math.cos(phi)) );

                mSphereColors.put((float) 0.8f );
                mSphereColors.put((float) 0.8f );
                mSphereColors.put((float) 0.8f );
                mSphereColors.put((float) 1.0f );

                mSphereNormals.put((float) (mRaduis * Math.sin(phi) * Math.cos(theta)) );
                mSphereNormals.put((float) (mRaduis * Math.sin(phi) * Math.sin(theta)) );
                mSphereNormals.put((float) (mRaduis * Math.cos(phi)) );
                points++;
            }
        }
        /*
        mSphereVertices.put(WorldLayoutData.CUBE_COORDS);
        mSphereNormals.put(WorldLayoutData.CUBE_NORMALS);
        mSphereColors.put(WorldLayoutData.CUBE_COLORS);
        */

        mSphereVertices.position(0);
        mSphereColors.position(0);
        mSphereNormals.position(0);
    }

    private void initSettings() {
        mCameraMat = new float[16];
        mViewMat = new float[16];
        mModelViewMat = new float[16];
        mModelViewProjectionMat = new float[16];
        mModelSphereMat = new float[16];
    }

    private void setupSphereProgram() {
        int vertexShader = MainActivity.loadGLShader(GLES20.GL_VERTEX_SHADER, R.raw.light_vertex);
        int gridShader = MainActivity.loadGLShader(GLES20.GL_FRAGMENT_SHADER, R.raw.grid_fragment);
        int passthroughShader = MainActivity.loadGLShader(GLES20.GL_FRAGMENT_SHADER, R.raw.passthrough_fragment);

        mSphereProgram = GLES20.glCreateProgram();
        GLES20.glAttachShader(mSphereProgram, vertexShader);
        GLES20.glAttachShader(mSphereProgram, passthroughShader);
        GLES20.glLinkProgram(mSphereProgram);
        GLES20.glUseProgram(mSphereProgram);

        mSpherePositionParam = GLES20.glGetAttribLocation(mSphereProgram, "a_Position");
        mSphereNormalParam = GLES20.glGetAttribLocation(mSphereProgram, "a_Normal");
        mSphereColorParam = GLES20.glGetAttribLocation(mSphereProgram, "a_Color");

        mSphereModelParam = GLES20.glGetUniformLocation(mSphereProgram, "u_Model");
        mSphereModelViewParam = GLES20.glGetUniformLocation(mSphereProgram, "u_MVMatrix");
        mSphereModelViewProjectionParam = GLES20.glGetUniformLocation(mSphereProgram, "u_MVP");
        mSphereLightPosParam = GLES20.glGetUniformLocation(mSphereProgram, "u_LightPos");

        GLES20.glEnableVertexAttribArray(mSpherePositionParam);
        GLES20.glEnableVertexAttribArray(mSphereNormalParam);
        GLES20.glEnableVertexAttribArray(mSphereColorParam);

        Matrix.setIdentityM(mModelSphereMat, 0);
    }

    public void drawSphere() {
        GLES20.glUseProgram(mSphereProgram);
        GLES20.glUniform3fv(mSphereLightPosParam, 1, lightPosInEyeSpace, 0);

        // Set the Model in the shader, used to calculate lighting
        GLES20.glUniformMatrix4fv(mSphereModelParam, 1, false, mModelSphereMat, 0);

        // Set the ModelView in the shader, used to calculate lighting
        GLES20.glUniformMatrix4fv(mSphereModelViewParam, 1, false, mModelViewMat, 0);

        // Set the position of the mSphere
        GLES20.glVertexAttribPointer(
                mSpherePositionParam, Params.COORDS_PER_VERTEX, GLES20.GL_FLOAT, false, 0, mSphereVertices);

        // Set the ModelViewProjection matrix in the shader.
        GLES20.glUniformMatrix4fv(mSphereModelViewProjectionParam, 1, false, mModelViewProjectionMat, 0);

        // Set the normal positions of the mSphere, again for shading
        GLES20.glVertexAttribPointer(mSphereNormalParam, 3, GLES20.GL_FLOAT, false, 0, mSphereNormals);
        GLES20.glVertexAttribPointer(mSphereColorParam, 4, GLES20.GL_FLOAT, false, 0, mSphereColors);

        GLES20.glDrawArrays(GLES20.GL_TRIANGLES, 0, 36);
    }

}
