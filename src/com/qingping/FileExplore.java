package com.qingping;

import java.io.File;
import java.io.FilenameFilter;
import java.util.ArrayList;

import android.app.Activity;
import android.app.ListActivity;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.Intent;
import android.opengl.GLSurfaceView;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.os.StatFs;
import android.text.format.Formatter;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.AdapterView.OnItemClickListener;
 
public class FileExplore extends ListActivity {
	
	private File[] files;
	private String[] names;
	private static int level = 0;
	private String dirPath;
	private ArrayAdapter<String> display;
	static{System.loadLibrary("dsoidneon");}
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		dirPath = Environment.getExternalStorageDirectory().getAbsolutePath();
		
		//dirPath = Environment.getDataDirectory().getAbsolutePath();
 
		ListView listView = getListView();
		listView.setTextFilterEnabled(true);
 
		listView.setOnItemClickListener(new OnItemClickListener() {
			public void onItemClick(AdapterView<?> parent, View view,
					int position, long id) {
				if(position == 0 && level != 0)
					goUp();
				else if(files[position].isDirectory())
					goDown(names[position]);
				else
				{
					    Toast.makeText(getApplicationContext(),
						((TextView) view).getText(), Toast.LENGTH_SHORT).show();
					    Log.i("dsoid", "touched");
					    startEmulation(files[position].getAbsolutePath());
				}
			}
		});
 
		
		loadFiles();
	}
	
	public void loadFiles()
	{
		findFiles();
		display = new ArrayAdapter<String>(this, R.layout.main,names);
		setListAdapter(display);
	}
	
	public void goUp()
	{
		level--;
		dirPath = dirPath.substring(0, dirPath.lastIndexOf(File.separator));
		
		findFiles();
		
		if(level > 0)
		{
			addEmpty();
		}
		
		display = new ArrayAdapter<String>(this, R.layout.main,names);
		setListAdapter(display);
	}
	
	public void goDown(String name)
	{
		level++;
		dirPath = dirPath+File.separator+name;
		
		findFiles();
		
		if(level > 0)
		{
			addEmpty();
		}
		
		display = new ArrayAdapter<String>(this, R.layout.main,names);
		setListAdapter(display);
	}
	
	public void addEmpty()
	{
		File[] tempFiles = new File[files.length + 1];
		System.arraycopy(files, 0, tempFiles, 1, files.length);
		files = tempFiles;
		
		String[] tempNames = new String[names.length + 1];
		System.arraycopy(names, 0, tempNames, 1, names.length);
		tempNames[0] = "/...";
		
		files = tempFiles;
		names = tempNames;
	}
	
	public void findFiles()
	{
		File mainDir = new File(dirPath);
		File[] roms = mainDir.listFiles(new FilenameFilter()
		    {
		        public boolean accept(File dir, String name)
		        {
		            return (name.endsWith(".nds") || name.endsWith(".zip"));
		        }
		    });
		
		File[] directories = mainDir.listFiles(new FilenameFilter()
	    {
	        public boolean accept(File dir, String name)
	        {
	        	File test = new File(dir.getAbsolutePath() + File.separator+name);
	            return test.isDirectory();
	        }
	    });
		
		files = new File[roms.length + directories.length];
		System.arraycopy(directories, 0, files, 0, directories.length);
		System.arraycopy(roms, 0, files, directories.length, roms.length);
		
		String[] romName = mainDir.list(new FilenameFilter()
		    {
		        public boolean accept(File dir, String name)
		        {
		        	return (name.endsWith(".nds") || name.endsWith(".zip"));
		        }
		    });
		
		String[] directoryName = mainDir.list(new FilenameFilter()
	    {
	        public boolean accept(File dir, String name)
	        {
	        	File test = new File(dir.getAbsolutePath() + File.separator+name);
	            return test.isDirectory();
	        }
	    });
		
		names = new String[romName.length + directoryName.length];
		System.arraycopy(directoryName, 0, names, 0, directoryName.length);
		System.arraycopy(romName, 0, names, directoryName.length, romName.length);
	}
	

	public void startEmulation(String path)
	{
		if(path.endsWith(".zip"))
		{
			ExtractFilesTask extract = new ExtractFilesTask(path, this);
			extract.execute();
		}
		else
		{
			Intent i = new Intent(this, EmulatorActivity.class);
		    i.putExtra("path", path);
		    startActivity(i); 
		}
	}
	
	private class ExtractFilesTask extends AsyncTask<Integer, Integer, Integer> 
	{
		private ProgressDialog dialog;
		private Context context;
		private String path;
		
		public ExtractFilesTask(String path, Context context)
		{
			super();
			dialog = new ProgressDialog(context);
			dialog.setCanceledOnTouchOutside(false);
			this.context=context;
			this.path=path;
		}

		@Override
	     protected void onPostExecute(Integer val) {
	         dialog.dismiss();
	         path = path.substring(0, path.length()-4) + ".nds";
	         Intent i = new Intent(context, EmulatorActivity.class);
			 i.putExtra("path", path);
			 startActivity(i); 
	     }
	     
		@Override
	     protected void onPreExecute()
	     {
			dialog.setMessage("Extracting file");
		    dialog.show();
	     }
	 
		@Override
		protected Integer doInBackground(Integer... arg0) {
			// TODO Auto-generated method stub
			Decompress decomp = new Decompress(path, path.substring(0, path.lastIndexOf(File.separator))+File.separator);
			Log.e("path", path+"  "+path.substring(0, path.lastIndexOf(File.separator))+File.separator);
			decomp.unzip();
			return null;
		}
	}

 
}