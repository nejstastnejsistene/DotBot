package com.thedotbot;

import java.io.File;
import java.io.IOException;

import com.thedotbot.screens.ScreenInfo;
import com.thedotbot.screens.ScreenReader;
import com.thedotbot.state.GameState;

import android.app.ActivityManager;
import android.app.ActivityManager.RunningTaskInfo;
import android.content.Context;
import android.util.Log;

public class Daemon implements Runnable {
	
	public static final String TAG = "Daemon";
	
	public static final String DOTS_PACKAGE = "com.nerdyoctopus.gamedots";
	public static final String SCREENSHOT_FILENAME = "screenshot.raw";

	private DaemonService service;
	private File screenshotFile;
	private GameState state;
	private boolean running;
	
	public Daemon(DaemonService service) {
		this.service = service;
		screenshotFile = new File(service.getFilesDir(), SCREENSHOT_FILENAME);
		state = null;
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
			if (!isDotsOpen()) {
				Log.d(TAG, "Dots not open, sleeping 5 seconds...");
				try {
					Thread.sleep(5000);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
				continue;
			}
			
			Log.d(TAG, "Dots is open!");
			runOnce();
			
			try {
				Thread.sleep(500);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}
	}
	
	public void runOnce() {
		try {
			Log.d(TAG, "Taking screenshot.");
			ScreenReader.takeScreenshot(screenshotFile);
			ScreenInfo info = ScreenReader.readScreen(screenshotFile);
			Log.d(TAG, "Screen info: " + info.getClass().getName());
			state = info.next(state);
		} catch (IOException e) {
			service.log(TAG, "Unable to take screenshot.", true);
			e.printStackTrace();
			stop();
			return;
		}
	}
	
	private boolean isDotsOpen() {
		ActivityManager am = (ActivityManager) service.getSystemService(Context.ACTIVITY_SERVICE);	
		RunningTaskInfo task = am.getRunningTasks(1).get(0);
		return task.topActivity.getPackageName().equals(DOTS_PACKAGE);
	}
	
}
