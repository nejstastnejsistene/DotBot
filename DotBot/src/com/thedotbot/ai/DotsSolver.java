package com.thedotbot.ai;

import com.thedotbot.screens.DotColor;

public class DotsSolver {
	
	public static int TARGET_SCORE = 300;

	public native static Move chooseMove(DotColor[][] board, int turnsRemaining, boolean allowTimeFreeze, boolean allowShrinkers, boolean allowExpander);
	public native static double scoreAbove(int turn, int score, int targetScore);
	
}
