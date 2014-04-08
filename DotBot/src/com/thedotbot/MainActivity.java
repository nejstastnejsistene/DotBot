package com.thedotbot;

import android.os.Bundle;
import android.app.Activity;
import android.content.Intent;
import android.view.Menu;
import android.view.View;

public class MainActivity extends Activity {

	public static final String TAG = "MainActivity";
	
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
    }


    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }
    
    public void startDaemon(View view) {
    	Intent intent = new Intent(this, DaemonService.class);
    	startService(intent);
    }
    
    public void stopDaemon(View view) {
    	Intent intent = new Intent(this, DaemonService.class);
    	stopService(intent);
    }
    
}
