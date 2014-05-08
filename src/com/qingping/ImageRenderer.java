package com.qingping;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.opengl.GLSurfaceView.Renderer;
import android.util.Log;

public class ImageRenderer implements Renderer
{
	private int width;
	private int height;
	
	
	public ImageRenderer(int w, int h)
	{
		super();
		this.width=w;
		this.height=h;
	}

	@Override
	public void onDrawFrame(GL10 arg0) {
		render();
		//Log.e("dsoid","Java render");
	}

	@Override
	public void onSurfaceChanged(GL10 arg0, int arg1, int arg2) 
	{
		initGL(width, height);
		Log.e("dsoid","opengl init");
		// TODO Auto-generated method stub
	}

	@Override
	public void onSurfaceCreated(GL10 arg0, EGLConfig arg1) 
	{
		// TODO Auto-generated method stub
		
	}
	
	private native double render();
	private native double initGL(double width, double height);
}
