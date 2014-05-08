package com.qingping;

import java.util.ArrayList;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Intent;
import android.opengl.GLSurfaceView;
import android.os.AsyncTask;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.view.View.OnTouchListener;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.RelativeLayout;

public class EmulatorActivity extends Activity
{
	GLSurfaceView glSurfaceView;
	ImageRenderer renderer;
	
	private native double entry(String path);  
	private native void deInitNDS();
	
	public native void setButtons(boolean left, boolean right, boolean up, boolean down, boolean start, boolean select, boolean a, boolean b, boolean x, boolean y, boolean l, boolean r);
	public native void touch(int x, int y);
	public native void releaseTouch();
	public native boolean haveNeon();
	
	public static native void emulationPause();
	public static native void emulationUnPause();
	
	static{System.loadLibrary("neondetect");}
	
	public static boolean first=true;
	
	private MenuItem load, save, settings;
	
	private boolean touching;
	private ArrayList<ImageButton> hiddenButtons;
	private ImageButton[] buttons;
	private boolean[] isPressed;
	private int[] pointerToView;
	private boolean fillHorizontal;
	private float scale;
	private float truncate;
	private float top;
	private float bottom;
	private float left;
	private float right;
	private ImageView img;
	
	public static native boolean isFinishedInit();
	
	@Override
	protected void onCreate(Bundle savedInstanceState) 
	{
		super.onCreate(savedInstanceState);
		
		if(haveNeon())
		{
			Log.e("loading library","loading neon");
			System.loadLibrary("dsoidneon");
		}
		else
		{	
			Log.e("loading library","no neon");
			System.loadLibrary("dsoidv7");
		}
			
		requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, 
                                WindowManager.LayoutParams.FLAG_FULLSCREEN);
        
        setContentView(R.layout.emulator); 
		
		hiddenButtons = new ArrayList<ImageButton>();
		buttons = new ImageButton[13];
		
		pointerToView = new int[10];
		for(int i=0;i<10;i++)
			pointerToView[i] = -1;
		
		buttons[0] = (ImageButton)findViewById(R.id.key_LEFT);  hiddenButtons.add(buttons[0]);
		buttons[1] = (ImageButton)findViewById(R.id.key_RIGHT);  hiddenButtons.add(buttons[1]);
		buttons[2] = (ImageButton)findViewById(R.id.key_UP);  hiddenButtons.add(buttons[2]);
		buttons[3] = (ImageButton)findViewById(R.id.key_DOWN);  hiddenButtons.add(buttons[3]);
		buttons[4] = (ImageButton)findViewById(R.id.key_START);  hiddenButtons.add(buttons[4]);
		buttons[5] = (ImageButton)findViewById(R.id.key_SELECT);  hiddenButtons.add(buttons[5]);
		buttons[6] = (ImageButton)findViewById(R.id.key_A);  hiddenButtons.add(buttons[6]);
		buttons[7] = (ImageButton)findViewById(R.id.key_B);  hiddenButtons.add(buttons[7]);
		buttons[8] = (ImageButton)findViewById(R.id.key_X);  hiddenButtons.add(buttons[8]);
		buttons[9] = (ImageButton)findViewById(R.id.key_Y);  hiddenButtons.add(buttons[9]);
		buttons[10] = (ImageButton)findViewById(R.id.key_L);  hiddenButtons.add(buttons[10]);
		buttons[11] = (ImageButton)findViewById(R.id.key_R);  hiddenButtons.add(buttons[11]);
		buttons[12] = (ImageButton)findViewById(R.id.key_touch); 
		
		isPressed = new boolean[12];
		
		GamepadListener glist = new GamepadListener();
		img = (ImageView) findViewById(R.id.touchCover);
		img.setOnTouchListener(glist);
		
		glSurfaceView=(GLSurfaceView) findViewById(R.id.screen); 
		DisplayMetrics metrics = new DisplayMetrics();
		getWindowManager().getDefaultDisplay().getMetrics(metrics);
		
		float height = metrics.heightPixels;
		float width = metrics.widthPixels;
        Log.e("dsoid", "width "+width+"height "+height);
        
        renderer = new ImageRenderer((int)width, (int)height);
        glSurfaceView.setRenderer(renderer);
        
        if(height/width < 384.0/256.0)
        {
        	fillHorizontal = false;
        	scale = (float)(height/384.0);
        	float gap = (float) (height * 256.0/768.0);
        	top = height/2;
        	left = gap;
        	bottom = height;
        	right = width-gap;
        }
        else
        {
        	fillHorizontal = true;
        	scale = (float) (width/256.0);
        	top = (float) (width * 384.0 / 512.0);
        	left = 0;
        	bottom = (float)(width *384.0/256.0);
        	right = width;
        }
		
        
        if(first)
		{
        	initCode();
        	first=false;
		}
        
        AsyncTask<Integer, Integer, Boolean> task = new AsyncTask<Integer, Integer, Boolean>()
        {
            ProgressDialog progressDialog;

            @Override
            protected void onPreExecute()
            {
                progressDialog = ProgressDialog.show(EmulatorActivity.this, "",
                        "Loading...");
            }

            @Override
            protected Boolean doInBackground(Integer... params)
            {
                while(!isFinishedInit()){};
                return true;
            }

            @Override
            protected void onPostExecute(Boolean result)
            {
              progressDialog.dismiss();
            }
        };
        
        
        task.execute();
        
	}
	
	@Override
	public boolean onCreateOptionsMenu(Menu menu)
	{
		super.onCreateOptionsMenu(menu);
		save = menu.add("Save");
		load = menu.add("Load");
		//settings = menu.add("Settings");
		emulationPause();
		return true;
	}
	
	@Override
	public boolean onPrepareOptionsMenu(Menu menu)
	{
		super.onPrepareOptionsMenu(menu);
		emulationPause();
		return true;
	}
	
	/*@Override
	public void onOptionsMenuClosed(Menu menu)
	{
		super.onOptionsMenuClosed(menu);
		Log.e("dsoid","closing options menu");
		emulationUnPause();
	}*/
	
	public native void saveState(int slotNum);
	public native void loadState(int slotNum);
	
	@Override
	public boolean onOptionsItemSelected(MenuItem item)
	{
		if(item == save)
		{
			saveState(1);
		}
		else if(item == load)
		{
			loadState(1);
		}
		else if(item == settings)
		{
			Intent i = new Intent(this, SettingsActivity.class);
		    startActivity(i); 
		}
		
		return false;
	}
	
	
	private boolean activityPaused=false;
	
	@Override
	protected void onResume()
	{
		super.onResume();
		if(glSurfaceView != null)
			glSurfaceView.onResume();
		
		if(activityPaused)
		{
			emulationUnPause();
			activityPaused=false;
		}
		
		Log.e("dsoid", "resuming");
	}
	
	@Override
	public void onBackPressed()
	{
		super.onBackPressed();
		Log.i("dsoid", "deInit");
		deInitNDS();
		first=true;
	}
	
	@Override
	protected void onPause()
	{		
	    super.onPause();
	    glSurfaceView.onPause();
	    
	    if(!activityPaused)
		{
			emulationPause();
			activityPaused=true;
		}
	    
	    Log.e("dsoid", "paused");
	}
	
	public void initCode()
	{
		Intent intent = getIntent();
		String path = intent.getExtras().getString("path");
		Log.e("dsoid", "starting emulation");
        entry(path);
	}
	
	public void enableTouch()
	{
		RelativeLayout lay = (RelativeLayout)findViewById(R.id.emulator_layout);
		//glSurfaceView.setOnTouchListener(new TouchpadOnTouchListener());
		for(int i=0;i<hiddenButtons.size();i++)
			lay.removeView(hiddenButtons.get(i));
		
		//lay.setOnTouchListener(new TouchpadOnTouchListener());
		
	}
	
	public void disableTouch()
	{
		RelativeLayout lay = (RelativeLayout)findViewById(R.id.emulator_layout);
		//glSurfaceView.setOnTouchListener(null);
		lay.removeView(img);
		for(int i=0;i<hiddenButtons.size();i++)
			lay.addView(hiddenButtons.get(i));
		
		//lay.setOnTouchListener(new GamepadListener());
		lay.addView(img);
		
	}
	
	public boolean inside(ImageButton butt, int x, int y)
	{
		int[] coord = new int[2];
		butt.getLocationOnScreen(coord);
		if(x>coord[0] && x<coord[0]+butt.getMeasuredWidth() && y>coord[1] && y < coord[1]+butt.getMeasuredHeight())
		{	
			return true;
		
		}
		
		return false;
	}
	

	private class GamepadListener implements OnTouchListener
	{
		public void press(int id)
		{
			isPressed[id] = true;
			setButtons(isPressed[0], isPressed[1], isPressed[2], isPressed[3], isPressed[4], isPressed[5], isPressed[6], isPressed[7], isPressed[8], isPressed[9], isPressed[10], isPressed[11]);
		}
		
		public void release(int id)
		{
			isPressed[id] = false;
			setButtons(isPressed[0], isPressed[1], isPressed[2], isPressed[3], isPressed[4], isPressed[5], isPressed[6], isPressed[7], isPressed[8], isPressed[9], isPressed[10], isPressed[11]);
		}
		
		public void processTouch(int buttonId, boolean isDown)
		{
			switch(buttonId)
			{
				case 12:
				{	
					if(!isDown)
					{
						if(!touching)
							enableTouch();
						else 
							disableTouch();
							
						touching = !touching;
					}
					
					break;
				}

				default:
				{
					if(!isDown)
						release(buttonId);
					else
						press(buttonId);
					
				}
			}	
		}
		
		public boolean onTouch(View view, MotionEvent event) 
		{
			if(!touching)
			{
				int mask=0;
				
				int x,y;
					
				x = (int)event.getX(event.getActionIndex());
				y = (int)event.getY(event.getActionIndex());
						
				for(int i=0;i<buttons.length;i++)
				{
					if(inside(buttons[i], x,y))
					{
						switch(event.getActionMasked())
						{
							case MotionEvent.ACTION_POINTER_UP:
							case MotionEvent.ACTION_CANCEL:
							case MotionEvent.ACTION_UP:
							{
								processTouch(i, false);
								int index = event.getActionIndex();
								pointerToView[event.getPointerId(index)] = -1;
								break;
							}
							
							case MotionEvent.ACTION_DOWN:
							case MotionEvent.ACTION_POINTER_DOWN:
							{
								processTouch(i, true);
								int index = event.getActionIndex();
								pointerToView[event.getPointerId(index)] = i;
								break;
							}
						}
					}
				}
				
				if(event.getActionMasked() == MotionEvent.ACTION_MOVE)
				{
					for(int i=0;i<event.getPointerCount();i++)
					{
						if(pointerToView[event.getPointerId(i)] != -1)
						{
							int tempx = (int)event.getX(i);
							int tempy = (int)event.getY(i);
							
							if(!inside(buttons[pointerToView[event.getPointerId(i)]], tempx, tempy))
							{	
								processTouch(pointerToView[event.getPointerId(i)], false);
								pointerToView[event.getPointerId(i)] = -1;
							}
						}
					}
				}
			}
			else
			{
				int x,y;
				
				x = (int)event.getX(event.getActionIndex());
				y = (int)event.getY(event.getActionIndex());
				
				if((event.getAction() == MotionEvent.ACTION_UP || event.getAction() == MotionEvent.ACTION_CANCEL || event.getActionMasked() == MotionEvent.ACTION_POINTER_UP) && inside(buttons[12], x, y))
				{
					if(!touching)
						enableTouch();
					else 
						disableTouch();
						
					touching = !touching;
					
					return true;
				}
				
				if(event.getX() >= left && event.getX() <= right && event.getY() >= top && event.getY() <= bottom)
				{
					float zeroX = (event.getX() - left)/scale;
					float zeroY = (event.getY() - top)/scale;
					
					Log.e("dsoid","touch coords x "+zeroX+"y "+zeroY);
					
					
					if(event.getAction() == MotionEvent.ACTION_DOWN || event.getAction() == MotionEvent.ACTION_MOVE)
					{
						Log.e("dsoid","set pointer");
						touch((int)zeroX, (int) zeroY);
						return true;
					}
					else if(event.getAction() == MotionEvent.ACTION_UP || event.getAction() == MotionEvent.ACTION_CANCEL)
					{	
						Log.e("dsoid", "touch released");
						releaseTouch();
						return true;
					}
					
					
				}
			}
				
			
			return true;
		}
	}
}
