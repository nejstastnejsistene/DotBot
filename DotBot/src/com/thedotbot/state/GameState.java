package com.thedotbot.state;

import com.thedotbot.screens.ScreenInfo;

public class GameState {

	private ScreenInfo screen;
	private int score;
	private boolean timeFreeze, shrinkers, expander;
	
	public void setScreen(ScreenInfo screen) {
		this.screen = screen;
	}
	
	public ScreenInfo getScreen() {
		return screen;
	}
	
	public int getScore() {
		return score;
	}
	
	public boolean hasTimeFreeze() {
		return timeFreeze;
	}
	
	public boolean useShrinkers() {
		return shrinkers;
	}
	
	public boolean hasExpander() {
		return expander;
	}
	
	public int getTurnsRemaining() {
		return -1;
	}
	
}
