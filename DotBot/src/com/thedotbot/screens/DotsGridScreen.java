package com.thedotbot.screens;

import java.util.Arrays;

import android.graphics.Point;

import com.thedotbot.ai.DotsSolver;
import com.thedotbot.ai.Move;
import com.thedotbot.state.GameState;
import com.thedotbot.state.MovesModeState;

public class DotsGridScreen implements ScreenInfo {

	private DotColor[][] board;
	private Point[][] coords;
	private Point menu, timeFreeze, expander;
	
	public Point getCoord(int row, int col) {
		return coords[col][row];
	}
	
	public Point getMenu() {
		return menu;
	}
	
	public Point getTimeFreeze() {
		return timeFreeze;
	}
	
	public Point getExpander() {
		return expander;
	}
	
	public boolean equals(Object obj) {
		if (!(obj instanceof DotsGridScreen)) {
			return false;
		}
		DotsGridScreen other = (DotsGridScreen)obj;
		return Arrays.equals(this.board, other.board);
	}
	
	@Override
	public GameState next(GameState state) {
		
		// If the screen isn't updated yet, don't do anything.
		if (!state.getScreen().equals(this)) {
			return state;
		}
		
		state.setScreen(this);
		
		Move move = DotsSolver.chooseMove(board, state.getTurnsRemaining(), state.hasTimeFreeze(), state.useShrinkers(), state.hasExpander());
		
		if (state instanceof MovesModeState) {
			MovesModeState mms = (MovesModeState)state;
			double p = DotsSolver.scoreAbove(mms.getTurn() + 1, mms.getScore() + move.getScore(), DotsSolver.TARGET_SCORE);
			if (p < 0.5) {
				// restart();
				// return new MenuState(...);
			}
			mms.nextTurn();
		}
		
		
		
		return state;
	}

}
