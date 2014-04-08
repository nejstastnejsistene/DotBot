package com.thedotbot;

import android.app.ActivityManager;
import android.app.ActivityManager.RunningTaskInfo;
import android.content.Context;
import android.util.Log;

public class Daemon implements Runnable {
	
	public static final String TAG = "Daemon";
	
	public static final String DOTS_PACKAGE = "com.nerdyoctopus.gamedots";

	private DaemonService service;
	private boolean running;
	
	public Daemon(DaemonService service) {
		this.service = service;
		running = false;
	}
	
	public boolean isRunning() {
		return running;
	}
	
	public void stop() {
		running = false;
	}
	
	public void run() {		
		running = true;
		while (running) {
			try {
				if (!isDotsOpen()) {
					Log.d(TAG, "Dots not open, sleeping 5 seconds...");
					Thread.sleep(5000);
					continue;
				}
				Log.d(TAG, "Dots is open!");
				Thread.sleep(500);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}
	}
	
	private boolean isDotsOpen() {
		ActivityManager am = (ActivityManager) service.getSystemService(Context.ACTIVITY_SERVICE);	
		RunningTaskInfo task = am.getRunningTasks(1).get(0);
		return task.topActivity.getPackageName().equals(DOTS_PACKAGE);
	}
	
}
