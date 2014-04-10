package com.thedotbot;

import java.io.DataOutputStream;
import java.io.IOException;

public class SuperShell {

	public static void su(String cmd) throws IOException {
		Process process = Runtime.getRuntime().exec("su");
		DataOutputStream os = new DataOutputStream(process.getOutputStream());	
		os.writeBytes(cmd + "\n");
		os.writeBytes("exit\n");
		os.flush();
		try {
			process.waitFor();
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
	}

}
