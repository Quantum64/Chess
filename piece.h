#pragma once

#include <string>
#include <vector>

#include "point.h"

class Game;
class PieceType {
private:
	std::string displayCharacter, name;
public:
	static const PieceType PAWN, ROOK, KNIGHT, BISHOP, QUEEN, KING, EMPTY;
	PieceType(std::string displayCharacter, std::string name) :
		displayCharacter(displayCharacter), name(name) {}
	bool operator==(PieceType piece) { return getDisplayCharacter() == piece.getDisplayCharacter(); }
	bool operator<(const PieceType& piece) const { return getDisplayCharacter().at(0) < piece.getDisplayCharacter().at(0); }
	std::string getDisplayCharacter() const { return displayCharacter; }
	std::string getName() { return name; }
};

enum class PieceColor {
	WHITE, BLACK
};

class Piece {
private:
	PieceType type;
	PieceColor color;
	bool firstMove = true;
	void lookInDirection(Game& game, std::vector<Point>& result, Point location, int right, int up, int max);
public:
	Piece() : type(PieceType::EMPTY), color(PieceColor::WHITE) {}
	Piece(PieceType type, PieceColor color) : type(type), color(color) {}
	bool operator==(Piece& piece) { return getType() == piece.getType() && getColor() == piece.getColor(); }
	PieceType getType() { return type; }
	PieceColor getColor() { return color; }
	std::vector<Point> getValidMoves(Game& game, Point location);
	std::vector<Point> getMoves(Game& game, Point location);
	bool isFirstMove() { return firstMove; }
	void setFirstMove(bool first) { firstMove = first; }
};