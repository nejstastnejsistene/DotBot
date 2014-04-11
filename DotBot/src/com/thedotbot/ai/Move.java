package com.thedotbot.ai;

import java.util.List;

import com.thedotbot.screens.DotColor;
import com.thedotbot.screens.DotsGridScreen;
import com.thedotbot.touch.TouchEvent;

public abstract class Move {

	private DotColor[][] newBoard;
	private int score;
	
	public Move(DotColor[][] newBoard, int score) {
		this.newBoard = newBoard;
		this.score = score;
	}
	
	public DotColor[][] getNewBoard() {
		return newBoard;
	}
	
	public int getScore() {
		return score;
	}
	
	public abstract List<TouchEvent> getTouchEvents(DotsGridScreen screen);
	
}
