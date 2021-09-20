    package com.example.webviewar;

import android.content.res.AssetManager;
import android.opengl.GLSurfaceView;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class Renderer implements GLSurfaceView.Renderer
{
    static {
        System.loadLibrary("gles3jni");
    }
    AssetManager globalAssets;

    public void setGlobalAssets(AssetManager assets) {
        globalAssets = assets;
    }

    public Renderer(AssetManager assets) {
        GLUtils.assets = assets;
        globalAssets = assets;
    }

    //GLUtils sharedShader;
    Texture sharedTexture = Texture.INSTANCE;

    native void nativeSurfaceChanged(int width, int height);

    native void nativeSurfaceCreated();

    native void nativeDrawFrame();

    @Override
    public void onSurfaceCreated(GL10 gl10, EGLConfig eglConfig)
    {
        sharedTexture.setAssets(globalAssets);
        nativeSurfaceCreated();
    }

    @Override
    public void onSurfaceChanged(GL10 gl10, int i, int i1)
    {
        nativeSurfaceChanged(i, i1);
    }

    @Override
    public void onDrawFrame(GL10 gl10)
    {
        nativeDrawFrame();
    }
}
