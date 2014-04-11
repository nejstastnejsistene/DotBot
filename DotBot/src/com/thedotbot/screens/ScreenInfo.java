package com.thedotbot.screens;

import com.thedotbot.state.GameState;

public interface ScreenInfo {

	public abstract GameState next(GameState state);
	
}
