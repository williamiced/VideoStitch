package com.gst_sdk_tutorials.tutorial_5;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.SurfaceTexture;
import android.media.MediaPlayer;
import android.opengl.GLES20;
import android.util.Log;
import android.view.MotionEvent;

import org.rajawali3d.materials.Material;
import org.rajawali3d.materials.textures.ATexture;
import org.rajawali3d.materials.textures.StreamingTexture;
import org.rajawali3d.materials.textures.Texture;
import org.rajawali3d.math.vector.Vector3;
import org.rajawali3d.primitives.Sphere;

/**
 * Created by wlee on 2016/4/28.
 */
public class MyRenderer extends VRRenderer {
    private Sphere mSphere;
    private StreamingTexture mSphereTexture;
    private StreamingTexture.ISurfaceListener mListener;

    public MyRenderer(Context context, StreamingTexture.ISurfaceListener listner) {
        super(context);

        mListener = listner;
    }

    public StreamingTexture getSphereTexture() {
        return mSphereTexture;
    }

    @Override
    protected void initScene() {
        //MediaPlayer mp = MediaPlayer.create(getContext(), R.raw.test);
        mSphereTexture = new StreamingTexture("SphereTexture", mListener);
        //mSphereTexture = new StreamingTexture("SphereTexture", mp);
        //mp.start();

        mSphere = createPhotoSphereWithTexture(mSphereTexture);
        getCurrentScene().addChild(mSphere);
        getCurrentCamera().setPosition(Vector3.ZERO);
        getCurrentCamera().setFieldOfView(100);
    }

    private Sphere createPhotoSphereWithTexture(ATexture texture) {
        Material material = new Material();
        material.setColor(0);

        try {
            material.addTexture(mTextureManager.addTexture(mSphereTexture));
        } catch (ATexture.TextureException e) {
            e.printStackTrace();
        }

        Sphere sphere = new Sphere(50, 64, 32);
        sphere.setScaleX(-1);
        sphere.setMaterial(material);

        return sphere;
    }

    @Override
    protected void onRender(long ellapsedRealtime, double deltaTime) {
        if (mSphereTexture != null)
            mSphereTexture.update();
        super.onRender(ellapsedRealtime, deltaTime);
    }

    @Override
    public void onOffsetsChanged(float xOffset, float yOffset, float xOffsetStep, float yOffsetStep, int xPixelOffset, int yPixelOffset) {

    }

    @Override
    public void onTouchEvent(MotionEvent event) {

    }
}
