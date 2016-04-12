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
import org.rajawali3d.math.vector.Vector3;
import org.rajawali3d.primitives.Sphere;

import java.util.ArrayList;

public class MyRenderer extends RajawaliCardboardRenderer {
    private Texture mSphereTexture;
    private Sphere mSphere;
    private Bitmap mT1;
    private Bitmap mT2;

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

    private static Sphere createPhotoSphereWithTexture(ATexture texture) {
        Material material = new Material();
        material.setColor(0);

        try {
            material.addTexture(texture);
        } catch (ATexture.TextureException e) {
            throw new RuntimeException(e);
        }

        Sphere sphere = new Sphere(50, 64, 32);
        sphere.setScaleX(-1);
        sphere.setMaterial(material);

        return sphere;
    }

    public void changeTextureByBitmap(Bitmap bm) {
        Texture t = (Texture) mSphere.getMaterial().getTextureList().get(0);
        t.setBitmap(bm);
        mSphere.getMaterial().getTextureList().set(0, t);
        this.reloadTextures();
    }

    public void changeImage(int resId) {
        long startTime = System.currentTimeMillis();
        long t1 = 0;
        long t2 = 0;
        long t3 = 0;
        long t4 = 0;
        long t5 = 0;
        Texture t;
        if (resId == R.drawable.panorama) {
            t1 = System.currentTimeMillis();
            t = (Texture) mSphere.getMaterial().getTextureList().get(0);
            t2 = System.currentTimeMillis();
            t.setBitmap(mT1);
            t3 = System.currentTimeMillis();
            mSphere.getMaterial().getTextureList().set(0, t);
            Log.d("MyRenderer", "Set to normal");
        } else  {
            t1 = System.currentTimeMillis();
            t = (Texture) mSphere.getMaterial().getTextureList().get(0);
            t2 = System.currentTimeMillis();
            t.setBitmap(mT2);
            t3 = System.currentTimeMillis();
            mSphere.getMaterial().getTextureList().set(0, t);
            Log.d("MyRenderer", "Set to bloody");
        }
        t4 = System.currentTimeMillis();
        this.reloadTextures();
        t5 = System.currentTimeMillis();
        Log.d("MyRenderer", "Time: " + (t1-startTime) + ", " + (t2-startTime) + ", " + (t3-startTime) + ", " + (t4-startTime) + ", " + (t5-startTime) );
    }
}