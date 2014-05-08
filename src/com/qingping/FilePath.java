package com.qingping;

import java.io.File;
import java.io.FilenameFilter;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Vector;

import android.os.Environment;
import android.util.Log;

public class FilePath 
{
	public static final String FOLDER_NAME="dsoid";
	
	public static String LoadModulePath()
	{
		String path=Environment.getExternalStorageDirectory().getAbsolutePath()+File.separator+FOLDER_NAME;
		File wallpaperDirectory = new File(path);
		wallpaperDirectory.mkdirs();
		return path;
	}

}
