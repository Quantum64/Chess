#pragma once
#include "constants.h"
#include "piece.h"

enum class BoardMode {
	DISPLAY,
	SELECT_PIECE,
	SELECT_TARGET
};

enum class GameState {
	PLAY,
	DRAW,
	WHITE_WIN,
	BLACK_WIN,
	WHITE_RESIGN,
	BLACK_RESIGN
};

class Game {
private:
	Piece pieces[BOARD_HEIGHT][BOARD_WIDTH];
	BoardMode mode = BoardMode::DISPLAY;
	Point selectedPiece = Point(0, 0), selectedTarget = Point(0, 0);
	Point lastSelected = Point(0, 0), lastTarget = Point(0, 0);
	PieceColor currentTurn = PieceColor::WHITE;
	bool firstMove = true, blackResigned = false, whiteResigned = false;
public:
	Game() {}
	Game(const Game& game);
	bool hasPiece(Point location) { return !(getPiece(location).getType() == PieceType::EMPTY); }
	Piece getPiece(Point location) { return pieces[location.y][location.x]; }
	PieceColor getCurrentTurn() { return currentTurn; }
	Point findNearestPiece(Point location, PieceColor color, int xOffset, int yOffset);
	GameState getState();
	void setSelectedPiece(Point point) { selectedPiece = point; }
	void setSelectedTarget(Point point) { selectedTarget = point; }
	void setCurrentTurn(PieceColor color) { currentTurn = color; }
	void reset();
	void startGame(bool ai);
	void draw(std::string help);
	bool selectPiece();
	bool selectTarget();
	void moveToTarget();
	void checkPawnUpgrade(bool ai);
	bool isInCheck(PieceColor color);
};