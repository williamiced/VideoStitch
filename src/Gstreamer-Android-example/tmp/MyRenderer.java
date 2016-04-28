package com.gst_sdk_tutorials.tutorial_5;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.media.MediaPlayer;
import android.util.Log;

import org.rajawali3d.cardboard.RajawaliCardboardRenderer;
import org.rajawali3d.materials.Material;
import org.rajawali3d.materials.textures.ATexture;
import org.rajawali3d.materials.textures.StreamingTexture;
import org.rajawali3d.materials.textures.Texture;
import org.rajawali3d.materials.textures.TextureManager;
import org.rajawali3d.math.vector.Vector3;
import org.rajawali3d.primitives.Sphere;

public class MyRenderer extends RajawaliCardboardRenderer {
    private Sphere mSphere;
    private StreamingTexture mSphereTexture;
    private Texture testTexture;
    private StreamingTexture.ISurfaceListener mListener;

    public MyRenderer(Context context) {
        super(context);
    }

    public MyRenderer(Context context, StreamingTexture.ISurfaceListener listener) {
        super(context);
        mListener = listener;
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
        super.onRender(ellapsedRealtime, deltaTime);
        if (mSphereTexture != null)
            mSphereTexture.update();
    }
}