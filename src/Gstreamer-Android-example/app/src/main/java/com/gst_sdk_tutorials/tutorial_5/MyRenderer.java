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
    private ArrayList<Bitmap> mTempBMList = null;
    private Bitmap mKeepBM = null;
    private boolean mShouldUpdateTexture = false;

    public MyRenderer(Context context) {
        super(context);
    }

    @Override
    protected void initScene() {
        mSphere = createPhotoSphereWithTexture(mSphereTexture);

        getCurrentScene().addChild(mSphere);

        getCurrentCamera().setPosition(Vector3.ZERO);
        getCurrentCamera().setFieldOfView(100);
    }

    private Sphere createPhotoSphereWithTexture(ATexture texture) {
        Material material = new Material();
        material.setColor(0);

        /*
        try {
            material.addTexture(mTextureManager.addTexture(mSphereTexture));
        } catch (ATexture.TextureException e) {
            throw new RuntimeException(e);
        }
        */

        Sphere sphere = new Sphere(50, 64, 32);
        sphere.setScaleX(-1);
        sphere.setMaterial(material);

        return sphere;
    }

    @Override
    protected void onRender(long ellapsedRealtime, double deltaTime) {
        if (mShouldUpdateTexture) {
            if (mKeepBM != null)
                mKeepBM.recycle();
            mKeepBM = mTempBMList.get(mTempBMList.size() - 1);

            // Recycle all
            for (int i=0; i<mTempBMList.size()-1; i++)
                mTempBMList.get(i).recycle();
            mTempBMList.clear();

            mSphereTexture.setBitmap(mKeepBM);
            mTextureManager.replaceTexture(mSphereTexture);

            mShouldUpdateTexture = false;
            Log.d("MyRender", "Update texture: " + mSphereTexture.getHeight() + ", " + mSphereTexture.getWidth());
        }
        super.onRender(ellapsedRealtime, deltaTime);
    }

    public void changeTextureByBitmap(Bitmap bm) {
        if (mTempBMList == null) {
            mTempBMList = new ArrayList<Bitmap>();
            mSphereTexture = new Texture("SphereTexture", bm);
            try {
                mSphere.getMaterial().addTexture(mTextureManager.addTexture(mSphereTexture));
            } catch (ATexture.TextureException e) {
                e.printStackTrace();
            }
            mTempBMList.add(bm);
        } else {
            mTempBMList.add(bm);
            mShouldUpdateTexture = true;
        }
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