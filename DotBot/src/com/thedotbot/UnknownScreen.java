package com.thedotbot;

public class UnknownScreen implements ScreenInfo {

	@Override
	public GameState next(GameState prevState) {
		return prevState;
	}

}
