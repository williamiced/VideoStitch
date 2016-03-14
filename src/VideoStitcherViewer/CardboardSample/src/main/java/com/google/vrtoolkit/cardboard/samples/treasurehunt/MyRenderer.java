package com.google.vrtoolkit.cardboard.samples.treasurehunt;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.opengl.GLES20;
import android.opengl.GLUtils;
import android.opengl.Matrix;
import android.util.Log;

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

    private int mRaduis = 1;

    private float[] mModelViewProjectionMat;
    private float[] mModelViewMat;
    private float[] mModelSphereMat;

    private int mSphereProgram;
    private int mSpherePositionParam;
    private int mSphereColorParam;
    private int mSphereNormalParam;

    private int mSphereModelViewParam;
    private int mSphereModelViewProjectionParam;
    private int mSphereLightPosParam;

    private FloatBuffer mSphereVertices;
    private FloatBuffer mSphereColors;
    private FloatBuffer mSphereNormals;
    private FloatBuffer mSphereTextures;

    private float[] mModelPosition;
    private int mPoints;

    private int mSphereTextureUniformHandle;
    private int mSphereTextureDataHandle;
    private int mSphereTextureParam;

    @Override
    public void onNewFrame(HeadTransform headTransform) {
        // Build the Model part of the ModelView matrix.
        //Matrix.rotateM(mModelSphereMat, 0, Params.TIME_DELTA, 0.5f, 0.5f, 1.0f);

        // Build the camera matrix and apply it to the ModelView.
        Matrix.setLookAtM(mCameraMat, 0, 0.0f, 0.0f, Params.CAMERA_Z, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    }

    @Override
    public void onDrawEye(Eye eye) {
        GLES20.glEnable(GLES20.GL_DEPTH_TEST);
        GLES20.glClearColor(1.0f, 1.0f, 1.0f, 0.5f);
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT | GLES20.GL_DEPTH_BUFFER_BIT);

        GLES20.glUseProgram(mSphereProgram);

        MainActivity.checkGLError("onDrawEye - clear");

        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, mSphereTextureDataHandle);
        GLES20.glUniform1i(mSphereTextureUniformHandle, 0);

        MainActivity.checkGLError("onDrawEye - texture");

        // Apply eyes transformation to camera
        Matrix.multiplyMM(mViewMat, 0, eye.getEyeView(), 0, mCameraMat, 0);

        // Set the position of the light
        Matrix.multiplyMV(lightPosInEyeSpace, 0, mViewMat, 0, Params.LIGHT_POS_IN_WORLD_SPACE, 0);

        float[] perspective = eye.getPerspective(Params.Z_NEAR, Params.Z_FAR);
        Matrix.multiplyMM(mModelViewMat, 0, mViewMat, 0, mModelSphereMat, 0);
        Matrix.multiplyMM(mModelViewProjectionMat, 0, perspective, 0, mModelViewMat, 0);

        MainActivity.checkGLError("onDrawEye - perspective");

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
        GLES20.glClearColor(1.0f, 1.0f, 1.0f, 0.5f);

        initBuffers();
        initSettings();
        setupSphereProgram();
        //updateModelPosition();

        mSphereTextureDataHandle = createTexture2D();
    }

    @Override
    public void onRendererShutdown() {

    }

    private void initBuffers() {
        int mStep = 5;
        double DEG = Math.PI/180;

        double dTheta = mStep * DEG;
        double dPhi = dTheta;
        int limit = (int) ((2 * Math.PI / dPhi + 1) * (2 * Math.PI / dTheta + 1) * 6 * 4);

        ByteBuffer bbVertices = ByteBuffer.allocateDirect(limit * 3);
        bbVertices.order(ByteOrder.nativeOrder());
        mSphereVertices = bbVertices.asFloatBuffer();

        ByteBuffer bbColors = ByteBuffer.allocateDirect(limit * 4);
        bbColors.order(ByteOrder.nativeOrder());
        mSphereColors = bbColors.asFloatBuffer();

        ByteBuffer bbNormals = ByteBuffer.allocateDirect(limit * 3);
        bbNormals.order(ByteOrder.nativeOrder());
        mSphereNormals = bbNormals.asFloatBuffer();

        ByteBuffer bbTextures = ByteBuffer.allocateDirect(limit * 2);
        bbTextures.order(ByteOrder.nativeOrder());
        mSphereTextures = bbTextures.asFloatBuffer();

        mPoints = 0;
        for(double phi = -(Math.PI); phi <= Math.PI; phi+=dPhi) {
            for(double theta = 0.0; theta <= (Math.PI * 2); theta+=dTheta) {
                double phi2 = phi + dPhi;
                double theta2 = theta + dTheta;
                if (phi2 > Math.PI)
                    phi2 = Math.PI;
                if (theta2 > Math.PI*2)
                    theta2 = Math.PI*2;
                double nP = (phi + phi2) / 2;
                double nT = (theta + theta2) / 2;

                putVertex(phi, theta, nP, nT);
                putVertex(phi2, theta, nP, nT);
                putVertex(phi, theta2, nP, nT);

                putVertex(phi2, theta, nP, nT);
                putVertex(phi2, theta2, nP, nT);
                putVertex(phi, theta2, nP, nT);

                mPoints += 6;
            }
        }

        Log.d("MyRender", "Points: " + mPoints);

        mSphereVertices.position(0);
        mSphereColors.position(0);
        mSphereNormals.position(0);
        mSphereTextures.position(0);

        MainActivity.checkGLError("initBuffers");
    }

    private void putVertex(double p, double t, double nP, double nT) {
        mSphereVertices.put((float) (mRaduis * Math.sin(p) * Math.cos(t)) );
        mSphereVertices.put((float) (mRaduis * Math.sin(p) * Math.sin(t)) );
        mSphereVertices.put((float) (mRaduis * Math.cos(p)) );

        mSphereColors.put(0.6f);
        mSphereColors.put(0.8f);
        mSphereColors.put(0.2f);
        mSphereColors.put(0.5f);

        mSphereNormals.put((float) (mRaduis * Math.sin(nP) * Math.cos(nT)) );
        mSphereNormals.put((float) (mRaduis * Math.sin(nP) * Math.sin(nT)) );
        mSphereNormals.put((float) (mRaduis * Math.cos(nP)) );

        mSphereTextures.put((float) (t / (2 * Math.PI)));
        mSphereTextures.put((float) ((p + Math.PI) / (2 * Math.PI)));

        //Log.d("TexTest", "Text: " + (t / (2*Math.PI)) + ", " + ( (p+Math.PI) / (2*Math.PI)));
    }

    private void initSettings() {
        mCameraMat = new float[16];
        mViewMat = new float[16];
        mModelViewMat = new float[16];
        mModelViewProjectionMat = new float[16];
        mModelSphereMat = new float[16];
        mModelPosition = new float[] {0.0f, 0.0f, -Params.MAX_MODEL_DISTANCE / 2.0f};
    }

    private void setupSphereProgram() {
        int vertexShader = MainActivity.loadGLShader(GLES20.GL_VERTEX_SHADER, R.raw.light_vertex);
        int passthroughShader = MainActivity.loadGLShader(GLES20.GL_FRAGMENT_SHADER, R.raw.passthrough_fragment);

        mSphereProgram = GLES20.glCreateProgram();
        GLES20.glAttachShader(mSphereProgram, vertexShader);
        GLES20.glAttachShader(mSphereProgram, passthroughShader);
        GLES20.glLinkProgram(mSphereProgram);
        GLES20.glUseProgram(mSphereProgram);

        MainActivity.checkGLError("compile program");

        mSphereTextureUniformHandle = GLES20.glGetUniformLocation(mSphereProgram, "u_Texture");

        mSpherePositionParam = GLES20.glGetAttribLocation(mSphereProgram, "a_Position");
        mSphereColorParam = GLES20.glGetAttribLocation(mSphereProgram, "a_Color");
        mSphereNormalParam = GLES20.glGetAttribLocation(mSphereProgram, "a_Normal");
        mSphereTextureParam = GLES20.glGetAttribLocation(mSphereProgram, "a_TexCoordinate");

        Log.d("ParamTest", "Position is : " + mSpherePositionParam);
        Log.d("ParamTest", "Color is : " + mSphereColorParam);
        Log.d("ParamTest", "Normal is : " + mSphereNormalParam);
        Log.d("ParamTest", "Texture is : " + mSphereTextureParam);

        MainActivity.checkGLError("set params");

        mSphereModelViewParam = GLES20.glGetUniformLocation(mSphereProgram, "u_MVMatrix");
        mSphereModelViewProjectionParam = GLES20.glGetUniformLocation(mSphereProgram, "u_MVPMatrix");
        mSphereLightPosParam = GLES20.glGetUniformLocation(mSphereProgram, "u_LightPos");

        MainActivity.checkGLError("set params2");

        GLES20.glEnableVertexAttribArray(mSpherePositionParam);
        GLES20.glEnableVertexAttribArray(mSphereColorParam);
        GLES20.glEnableVertexAttribArray(mSphereNormalParam);
        GLES20.glEnableVertexAttribArray(mSphereTextureParam);

        Matrix.setIdentityM(mModelSphereMat, 0);

        MainActivity.checkGLError("setupSphereProgram");
    }

    public void drawSphere() {
        GLES20.glUniform3fv(mSphereLightPosParam, 1, lightPosInEyeSpace, 0);
        GLES20.glUniformMatrix4fv(mSphereModelViewParam, 1, false, mModelViewMat, 0);
        GLES20.glUniformMatrix4fv(mSphereModelViewProjectionParam, 1, false, mModelViewProjectionMat, 0);

        MainActivity.checkGLError("drawSphere - uniform");

        GLES20.glVertexAttribPointer(mSpherePositionParam, 3, GLES20.GL_FLOAT, false, 0, mSphereVertices);
        GLES20.glVertexAttribPointer(mSphereColorParam, 4, GLES20.GL_FLOAT, false, 0, mSphereColors);
        GLES20.glVertexAttribPointer(mSphereNormalParam, 3, GLES20.GL_FLOAT, false, 0, mSphereNormals);
        GLES20.glVertexAttribPointer(mSphereTextureParam, 2, GLES20.GL_FLOAT, false, 0, mSphereTextures);

        GLES20.glDrawArrays(GLES20.GL_TRIANGLES, 0, mPoints);

        MainActivity.checkGLError("drawSphere - params");
    }

    private void updateModelPosition() {
        Matrix.setIdentityM(mModelSphereMat, 0);
        Matrix.translateM(mModelSphereMat, 0, mModelPosition[0], mModelPosition[1], mModelPosition[2]);
    }

    private int createTexture2D() {
        // Texture object handle
        int[] textureId = new int[1];
        try {
            final BitmapFactory.Options options = new BitmapFactory.Options();
            options.inScaled = false;   // No pre-scaling

            Bitmap bitmap = BitmapFactory.decodeResource(MainActivity.getContext().getResources(), R.drawable.tmp);
            // Generate a texture object
            GLES20.glGenTextures(1, textureId, 0);
            // Bind the texture object
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureId[0]);

            GLUtils.texImage2D(GLES20.GL_TEXTURE_2D, 0, bitmap, 0);
            // Set the filtering mode
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D,
                    GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D,
                    GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
            bitmap.recycle();
        } catch (Exception e) {
            e.printStackTrace();
            Log.d("Texture Test", "Load texture failed");
        } finally {
        }
        MainActivity.checkGLError("createTexture2D");
        return textureId[0];
    }


}