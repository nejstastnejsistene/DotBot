package com.thedotbot.screens;

import com.thedotbot.state.GameState;

public class UnknownScreen implements ScreenInfo {

	@Override
	public GameState next(GameState state) {
		return state;
	}

}
