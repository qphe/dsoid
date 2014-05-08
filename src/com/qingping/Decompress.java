package com.qingping;

import android.util.Log; 
import java.io.File; 
import java.io.FileInputStream; 
import java.io.FileOutputStream; 
import java.util.zip.ZipEntry; 
import java.util.zip.ZipInputStream; 
 
public class Decompress { 
  private String _zipFile; 
  private String _location; 
 
  public Decompress(String zipFile, String location) { 
    _zipFile = zipFile; 
    _location = location; 
 
    _dirChecker(""); 
  } 
 
  public void unzip() { 
    try  { 
      FileInputStream fin = new FileInputStream(_zipFile); 
      ZipInputStream zin = new ZipInputStream(fin); 
      ZipEntry ze = null; 
      while ((ze = zin.getNextEntry()) != null) { 
        Log.v("Decompress", "Unzipping " + ze.getName()); 
 
        if(ze.isDirectory()) { 
          _dirChecker(ze.getName()); 
        } else { 
          FileOutputStream fout = new FileOutputStream(_location + ze.getName()); 
          
          
          byte[] buffer = new byte[4096*256]; 
          
          for (int c = zin.read(buffer); c != -1; c = zin.read(buffer)) 
          { fout.write(buffer, 0, c); }
 
          zin.closeEntry(); 
          fout.close(); 
        } 
         
      } 
      zin.close(); 
    } catch(Exception e) { 
      Log.e("Decompress", "unzip", e); 
    } 
 
  } 
 
  private void _dirChecker(String dir) { 
    File f = new File(_location + dir); 
 
    if(!f.isDirectory()) { 
      boolean success = f.mkdirs();
      if (success)
    	  Log.e("Create directory", "Succeeded");
      else
    	  Log.e("Create directory", "Failed");  
    } 
  } 
} 

