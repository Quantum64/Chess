#include <functional>
#include <algorithm>
#include <chrono>

#include "piece.h"
#include "game.h"
#include "console.h"

const PieceType PieceType::PAWN = PieceType("P", "Pawn");
const PieceType PieceType::ROOK = PieceType("R", "Rook");
const PieceType PieceType::KNIGHT = PieceType("N", "Knight");
const PieceType PieceType::BISHOP = PieceType("B", "Bishop");
const PieceType PieceType::QUEEN = PieceType("Q", "Queen");
const PieceType PieceType::KING = PieceType("K", "King");
const PieceType PieceType::EMPTY = PieceType(" ", "");

bool isValidLocation(int x, int y) {
	if (x < 0 || x >= BOARD_WIDTH) {
		return false;
	}
	if (y < 0 || y >= BOARD_HEIGHT) {
		return false;
	}
	return true;
}

void Piece::lookInDirection(Game& game, std::vector<Point>& result, Point location, int right, int up, int max) {
	if (right == 0 && up == 0) {
		return;
	}
	int x = location.x, y = location.y, count = 0;
	while (true) {
		if (max > 0 && count >= max) {
			break;
		}
		count++;
		x += right;
		y += up;
		if (!isValidLocation(x, y)) {
			break;
		}
		Point point(x, y);
		if (!game.hasPiece(point)) {
			result.push_back(point);
			continue;
		}
		Piece piece = game.getPiece(point);
		if (piece.getColor() != getColor()) {
			result.push_back(point);
		}
		break;
	}
}

void runForValidOffset(Point location, int offsetX, int offsetY, std::function<void(Point loc)> action) {
	int x = location.x + offsetX, y = location.y + offsetY;
	if (isValidLocation(x, y)) {
		action(Point(x, y));
	}
}

std::vector<Point> Piece::getValidMoves(Game& game, Point location) {
	std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
	std::vector<Point> result = getMoves(game, location);
	std::vector<Point> checked;
	std::copy_if(result.begin(), result.end(), std::back_inserter(checked), [&](Point point) {
		Game copy = game;
		copy.setSelectedPiece(location);
		copy.setSelectedTarget(point);
		copy.moveToTarget();
		return !copy.isInCheck(getColor());
	});
	std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
	std::string debug = "Calculating valid moves took ";
	debug += std::to_string(duration);
	debug += " microseconds (" + std::to_string(duration / 1000) + " milliseconds).\n";
	getConsole().debug(debug);
	return checked;
}

std::vector<Point> Piece::getMoves(Game& game, Point location) {
	std::vector<Point> result;
	auto pawnLogicCapture = [&](Point loc) {
		if (game.hasPiece(loc)) {
			if (game.getPiece(loc).getColor() != getColor()) {
				result.push_back(loc);
			}
		}
	};
	auto pawnLogicForward = [&](Point loc) {
		if (!game.hasPiece(loc)) {
			result.push_back(loc);
		}
	};
	auto pawnLogicForwardFirst = [&](Point loc) {
		if (isFirstMove()) {
			if (game.hasPiece(Point(loc.x, loc.y + (getColor() == PieceColor::WHITE ? 1 : -1)))) {
				return;
			}
			if (!game.hasPiece(loc)) {
				result.push_back(loc);
			}
		}
	};
	auto kingCastleLogic = [&](Point loc) {
		if (isFirstMove()) {
			Point rookRight(loc.x + 3, loc.y);
			if (game.hasPiece(rookRight)) {
				if (game.getPiece(rookRight).isFirstMove()) {
					if (!game.hasPiece(Point(loc.x + 2, loc.y)) && !game.hasPiece(Point(loc.x + 1, loc.y))) {
						result.push_back(Point(loc.x + 2, loc.y));
					}
				}
			}
			Point rookLeft(loc.x - 4, loc.y);
			if (game.hasPiece(rookLeft)) {
				if (game.getPiece(rookLeft).isFirstMove()) {
					if (!game.hasPiece(Point(loc.x - 3, loc.y)) && !game.hasPiece(Point(loc.x - 2, loc.y)) && !game.hasPiece(Point(loc.x - 1, loc.y))) {
						result.push_back(Point(loc.x - 2, loc.y));
					}
				}
			}
		}
	};
	if (getType() == PieceType::PAWN && getColor() == PieceColor::WHITE) {
		runForValidOffset(location, 0, -1, pawnLogicForward);
		runForValidOffset(location, 0, -2, pawnLogicForwardFirst);
		runForValidOffset(location, 1, -1, pawnLogicCapture);
		runForValidOffset(location, -1, -1, pawnLogicCapture);
	}
	else if (getType() == PieceType::PAWN && getColor() == PieceColor::BLACK) {
		runForValidOffset(location, 0, 1, pawnLogicForward);
		runForValidOffset(location, 0, 2, pawnLogicForwardFirst);
		runForValidOffset(location, 1, 1, pawnLogicCapture);
		runForValidOffset(location, -1, 1, pawnLogicCapture);
	}
	else if (getType() == PieceType::ROOK) {
		int options[4][2] = { {1, 0}, { -1, 0 }, {0, 1 }, {0, -1} };
		for (int* r : options) {
			lookInDirection(game, result, location, r[0], r[1], -1);
		}
	}
	else if (getType() == PieceType::BISHOP) {
		int options[4][2] = { {1, 1}, { -1, -1 }, {-1, 1 }, {1, -1} };
		for (int* r : options) {
			lookInDirection(game, result, location, r[0], r[1], -1);
		}
	}
	else if (getType() == PieceType::QUEEN) {
		int options[8][2] = { {1, 1}, { -1, -1 }, {-1, 1 }, {1, -1}, {1, 0}, { -1, 0 }, {0, 1 }, {0, -1} };
		for (int* r : options) {
			lookInDirection(game, result, location, r[0], r[1], -1);
		}
	}
	else if (getType() == PieceType::KNIGHT) {
		int options[8][2] = { {1, 2}, {1, -2}, {-1, 2}, {-1, -2}, {2, 1}, {2, -1}, {-2, 1}, {-2, -1} };
		for (int* r : options) {
			runForValidOffset(location, r[0], r[1], [&](Point loc) {
				if (game.hasPiece(loc)) {
					if (game.getPiece(loc).getColor() == getColor()) {
						return;
					}
				}
				result.push_back(loc);
			});
		}
	}
	else if (getType() == PieceType::KING) {
		int options[8][2] = { {1, 1}, { -1, -1 }, {-1, 1 }, {1, -1}, {1, 0}, { -1, 0 }, {0, 1 }, {0, -1} };
		for (int* r : options) {
			lookInDirection(game, result, location, r[0], r[1], 1);
		}
		runForValidOffset(location, 0, 0, kingCastleLogic);
	}
	return result;
}