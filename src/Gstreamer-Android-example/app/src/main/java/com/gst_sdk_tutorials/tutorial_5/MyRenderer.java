package com.gst_sdk_tutorials.tutorial_5;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.util.Log;

import org.rajawali3d.Object3D;
import org.rajawali3d.cardboard.RajawaliCardboardRenderer;
import org.rajawali3d.materials.Material;
import org.rajawali3d.materials.textures.ATexture;
import org.rajawali3d.materials.textures.Texture;
import org.rajawali3d.materials.textures.TextureManager;
import org.rajawali3d.math.vector.Vector3;
import org.rajawali3d.primitives.Sphere;

import java.util.ArrayList;

public class MyRenderer extends RajawaliCardboardRenderer {
    private Texture mSphereTexture;
    private Sphere mSphere;
    private Bitmap mT1;
    private Bitmap mT2;
    private Bitmap mBM = null;
    private boolean mShouldUpdateTexture = false;

    public MyRenderer(Context context) {
        super(context);
    }

    @Override
    protected void initScene() {
        mSphereTexture = new Texture("photo", R.drawable.panorama);
        mSphere = createPhotoSphereWithTexture(mSphereTexture);

        mT1 = BitmapFactory.decodeResource(getContext().getResources(), R.drawable.panorama);
        mT2 = BitmapFactory.decodeResource(getContext().getResources(), R.drawable.panorama_bloody);

        getCurrentScene().addChild(mSphere);

        getCurrentCamera().setPosition(Vector3.ZERO);
        getCurrentCamera().setFieldOfView(100);
    }

    private Sphere createPhotoSphereWithTexture(ATexture texture) {
        Material material = new Material();
        material.setColor(0);

        try {
            material.addTexture(mTextureManager.addTexture(texture));
        } catch (ATexture.TextureException e) {
            throw new RuntimeException(e);
        }

        Sphere sphere = new Sphere(50, 64, 32);
        sphere.setScaleX(-1);
        sphere.setMaterial(material);

        return sphere;
    }

    @Override
    protected void onRender(long ellapsedRealtime, double deltaTime) {
        if (mShouldUpdateTexture) {
            mSphereTexture.setBitmap(mBM);
            mTextureManager.replaceTexture(mSphereTexture);
            mShouldUpdateTexture = false;
        }
        super.onRender(ellapsedRealtime, deltaTime);
    }

    public void changeTextureByBitmap(Bitmap bm) {
        Bitmap lastBM = mBM;
        mBM = bm;
        if (lastBM != null)
            lastBM.recycle();
        mShouldUpdateTexture = true;
        //mSphereTexture.setBitmap(bm);
        //mTextureManager.replaceTexture(mSphereTexture);
        //this.reloadTextures();
        /*
        Texture t = (Texture) mSphere.getMaterial().getTextureList().get(0);
        t.setBitmap(bm);
        mSphere.getMaterial().getTextureList().set(0, t);
        this.reloadTextures();
        */
    }
}