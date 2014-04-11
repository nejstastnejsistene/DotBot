package com.thedotbot.state;


public class MovesModeState extends GameState {
	
	private int turn;
	private int turnsRemaining;
	
	public void addTurns(int amount) {
		turnsRemaining += amount;
	}
	
	public void nextTurn() {
		turn++;
		turnsRemaining--;
	}

	public int getTurn() {
		return turn;
	}
	
	@Override
	public int getTurnsRemaining() {
		return turnsRemaining;
	}
	
}
