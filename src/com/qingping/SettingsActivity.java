package com.qingping;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.CheckBox;

public class SettingsActivity extends Activity
{
	private CheckBox chkSound;
	 
	@Override
	public void onCreate(Bundle savedInstanceState) 
	{
		EmulatorActivity.emulationPause();
		
		super.onCreate(savedInstanceState);
		setContentView(R.layout.settings);
		
		SettingsOnClickListener listen = new SettingsOnClickListener();
		
		chkSound = (CheckBox) findViewById(R.id.chkSound);
		chkSound.setOnClickListener(listen);
	}
	
	@Override
	public void onBackPressed()
	{
		EmulatorActivity.emulationUnPause();
		
		super.onBackPressed();
	}
	
	private native void enableSound();
	private native void disableSound();
	  
	private class SettingsOnClickListener implements OnClickListener
	{
		@Override
		public void onClick(View v) 
		{
	                //is chkIos checked?
			if (v == chkSound) 
			{
				if(((CheckBox) v).isChecked())
					enableSound();
				else
					disableSound();
			}
	 
		}
	}
}
