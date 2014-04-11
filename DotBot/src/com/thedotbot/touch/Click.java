package com.thedotbot.touch;

import android.graphics.Point;

public class Click implements TouchEvent {

	private Point p;
	
	public Click(Point p) {
		this.p = p;
	}
	
	public String toString() {
		return "click " + p.x + " " + p.y + "\n";
	}
	
}
