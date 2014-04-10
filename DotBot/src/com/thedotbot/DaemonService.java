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
			log(TAG, "Daemon is already running!", false);
			return;
		}
		log(TAG, "Daemon started.", false);
		new Thread(daemon).start();
	}

	
	@Override
	public void onDestroy() {
		daemon.stop();
		log(TAG, "Daemon stopped.", false);
	}
	
	
	public void log(final String tag, final String message, final boolean error) {
		final DaemonService service = this;
		handler.post(new Runnable() {
			public void run() {
				if (error) {
					Log.e(tag, message);
				} else {
					Log.i(tag, message);
				}
				Toast.makeText(service, message, Toast.LENGTH_SHORT).show();
			}
		});
	}

}
