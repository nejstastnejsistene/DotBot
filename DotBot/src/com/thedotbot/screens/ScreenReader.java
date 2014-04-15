package com.thedotbot.screens;

import java.io.File;
import java.io.IOException;

import com.thedotbot.SuperShell;

public class ScreenReader {

	public static void takeScreenshot(File file) throws IOException {
		SuperShell.su("/system/bin/screencap " + file.getPath());
		SuperShell.su("/system/bin/chmod 644 " + file.getPath());
	}
	
	public static native ScreenInfo readScreen(File file);
	
}
