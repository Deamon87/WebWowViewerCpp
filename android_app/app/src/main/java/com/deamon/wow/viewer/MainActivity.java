package com.deamon.wow.viewer;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import android.content.res.AssetManager;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.SurfaceHolder;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class MainActivity extends AppCompatActivity {

    private GLSurfaceView glSurfaceView;
    boolean m_intialized = false;

    long m_lastTime = 0;


    static {
        System.loadLibrary("AWebWoWViewerCpp");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
//        setContentView(R.layout.activity_main);

        glSurfaceView = new GLSurfaceView(this);
        glSurfaceView.setEGLContextClientVersion(2);
        glSurfaceView.setRenderer(new GLSurfaceView.Renderer() {

            @Override
            public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            }

            @Override
            public void onSurfaceChanged(GL10 gl, int width, int height) {
                if (!m_intialized) {
                    createScene(width, height, glSurfaceView.getHolder().getSurface(), getAssets());
                    m_intialized = true;
                }
            }

            @Override
            public void onDrawFrame(GL10 gl) {
                long nowTime = System.nanoTime();
                if (m_lastTime == 0) {
                    m_lastTime = nowTime;
                }
                double delta = (float)(nowTime - m_lastTime) / 1000000000.0;
                gameloop(delta);


                m_lastTime = nowTime;
            }
        });
        setContentView(glSurfaceView);
    }

    public native void createScene(int canvWidth, int canvHeight, android.view.Surface surface, AssetManager assetManager);
    public native void gameloop(double deltaTime);


}