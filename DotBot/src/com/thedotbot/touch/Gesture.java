package com.thedotbot.touch;

import java.util.List;

import android.graphics.Point;

public class Gesture implements TouchEvent {

	private List<Point> points;
	
	public Gesture(List<Point> points) {
		this.points = points;
	}
	
	public String toString() {
		StringBuilder s = new StringBuilder("gesture");
		for (Point p : points) {
			s.append(" " + p.x + " " + p.y);
		}
		s.append("\n");
		return s.toString();
	}
}
