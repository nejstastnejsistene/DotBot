package com.thedotbot.ai;

import java.util.ArrayList;
import java.util.List;

import com.thedotbot.screens.DotColor;
import com.thedotbot.screens.DotsGridScreen;
import com.thedotbot.touch.Click;
import com.thedotbot.touch.TouchEvent;

public class Shrinker extends Move {
	
	private int row, col;
	
	public Shrinker(DotColor[][] newBoard, int score, int row, int col) {
		super(newBoard, score);
		this.row = row;
		this.col = col;
	}
	
	@Override
	public List<TouchEvent> getTouchEvents(DotsGridScreen screen) {
		Click click = new Click(screen.getCoord(row, col));
		ArrayList<TouchEvent> events = new ArrayList<TouchEvent>(2);
		events.add(click);
		events.add(click);
		return events;
	}

}
