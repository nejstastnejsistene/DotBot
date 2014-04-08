package com.thedotbot;

import android.app.Service;
import android.content.Intent;
import android.os.Handler;
import android.os.IBinder;
import android.util.Log;
import android.widget.Toast;

public class DaemonService extends Service {

	public static final String TAG = "DaemonService";
	
	private Daemon daemon;
	private Handler handler;
	
	@Override
	public IBinder onBind(Intent intent) {
		return null;
	}
	
	@Override
	public void onCreate() {
		daemon = new Daemon(this);
		handler = new Handler();
	}
	
	@Override
	public void onStart(Intent intent, int startId) {
		if (daemon.isRunning()) {
			info("Daemon is already running!");
			return;
		}
		info("Daemon started.");
		new Thread(daemon).start();
	}

	
	@Override
	public void onDestroy() {
		daemon.stop();
		info("Daemon stopped.");
	}
	
	private void info(String message) {
		info(TAG, message);
	}
	
	public void info(final String tag, final String message) {
		final DaemonService service = this;
		handler.post(new Runnable() {
			public void run() {
				Log.i(tag, message);
				Toast.makeText(service, message, Toast.LENGTH_SHORT).show();
			}
		});
	}

}
