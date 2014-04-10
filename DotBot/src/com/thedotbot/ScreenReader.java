package com.thedotbot;

import java.io.File;
import java.io.IOException;

public class ScreenReader {

	public static void takeScreenshot(File file) throws IOException {
		SuperShell.su("/system/bin/screencap " + file.getPath());
	}
	
	public static native ScreenInfo readScreen(File file);
	
}
