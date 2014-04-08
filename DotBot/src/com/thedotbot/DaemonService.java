package com.thedotbot;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.util.Log;
import android.widget.Toast;

public class DaemonService extends Service {

	public static final String TAG = "DaemonService";
	
	private boolean running;
	
	@Override
	public IBinder onBind(Intent intent) {
		return null;
	}
	
	@Override
	public void onCreate() {
		running = false;
	}
	
	@Override
	public void onStart(Intent intent, int startId) {
		if (running) {
			Toast.makeText(getApplicationContext(), "Daemon is already running!", Toast.LENGTH_SHORT).show();
			return;
		}
		running = true;
		Log.i(TAG, "Daemon started.");
		Toast.makeText(getApplicationContext(), "Daemon started.", Toast.LENGTH_SHORT).show();
		
		// Game loop here 
	}

	
	@Override
	public void onDestroy() {
		running = false;
		Log.i(TAG, "Daemon stopped.");
		Toast.makeText(getApplicationContext(), "Daemon stopped.", Toast.LENGTH_SHORT).show();
	}

}
