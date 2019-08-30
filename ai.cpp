#include <algorithm>

#include "game.h"
#include "ai.h"
#include "constants.h"

// This AI is "greedy", meaning that it only looks at the effect that its next
// move will have on the state of the game. It does this by looking at how each
// possible move will affect how its pieces are being attacked by the other side,
// then it adds the possible effects of pieces it can take, plus how many of the
// opponent's pieces it can endanger with the next move, then chooses the move
// that minimises its losses and maximizes its gains. It is easy to defeat with
// a bit of planning since it has no concent of checkmate, how to achieve it, or
// how to defend it. It is, however, adequate for this project since an AI is
// optional and isn't really the focus of the project.

class PossibleMove {
public:
	Point from, to;
	Piece piece;
	int material;
	bool defensive;
	PossibleMove() : from(Point(0, 0)), to(Point(0, 0)) {}
	PossibleMove(Point from, Point to, int material, bool defensive) : from(from), to(to), material(material), defensive(defensive) {}
};

int getMaterialValue(PieceType type) {
	if (type == PieceType::PAWN) {
		return 1;
	}
	else if (type == PieceType::BISHOP || type == PieceType::KNIGHT) {
		return 3;
	}
	else if (type == PieceType::ROOK) {
		return 5;
	}
	else if (type == PieceType::QUEEN) {
		return 9;
	}
	else if (type == PieceType::KING) {
		return 3; // Non-standard, but gives the AI a bit of incentive to put the player in check
	}
	else {
		return 0;
	}
}

int getEndangeredMaterial(Game& game, PieceColor color) {
	int endangeredMaterial = 0;
	for (int y = 0; y < BOARD_WIDTH; y++) {
		for (int x = 0; x < BOARD_HEIGHT; x++) {
			Point point(x, y);
			if (game.hasPiece(point)) {
				Piece piece = game.getPiece(point);
				if (piece.getColor() != color) {
					for (Point to : piece.getValidMoves(game, point)) {
						if (game.hasPiece(to)) {
							Piece taken = game.getPiece(to);
							if (taken.getColor() == color) {
								endangeredMaterial += getMaterialValue(taken.getType());
							}
						}
					}
				}
			}
		}
	}
	return endangeredMaterial;
}

void aiMakeMove(Game& game) {
	// Calculate how much material the other player can claim here first
	// so we can use it to compare to possible moves later
	int endangeredMaterial = getEndangeredMaterial(game, game.getCurrentTurn());
	int attackingMaterial = getEndangeredMaterial(game, game.getCurrentTurn() == PieceColor::WHITE ? PieceColor::BLACK : PieceColor::WHITE);
	std::vector<PossibleMove> moves;
	for (int y = 0; y < BOARD_WIDTH; y++) {
		for (int x = 0; x < BOARD_HEIGHT; x++) {
			Point point(x, y);
			if (game.hasPiece(point)) {
				Piece piece = game.getPiece(point);
				if (piece.getColor() == game.getCurrentTurn()) {
					for (Point to : piece.getValidMoves(game, point)) {
						// Look at the other color's possible moves here to see how the total material
						// that the other player can claim will change after this move. If negative,
						// assign defensive move value
						// TODO: Investigate how slow this is
						Game copy = game;
						copy.setSelectedPiece(point);
						copy.setSelectedTarget(to);
						copy.moveToTarget();
						int updatedEndangered = getEndangeredMaterial(copy, game.getCurrentTurn());
						int updatedAttacking = getEndangeredMaterial(copy, game.getCurrentTurn() == PieceColor::WHITE ? PieceColor::BLACK : PieceColor::WHITE);
						int materialChange = endangeredMaterial - updatedEndangered;
						int attackingChange = (updatedAttacking - attackingMaterial) / 3; // What effect does this really have?
						materialChange += attackingChange;
						if (game.hasPiece(to)) {
							Piece taken = game.getPiece(to);
							if (taken.getColor() != game.getCurrentTurn()) {
								materialChange += getMaterialValue(taken.getType());
							}
						}
						PossibleMove move(point, to, materialChange, updatedEndangered < endangeredMaterial);
						moves.push_back(move);
					}
				}
			}
		}
	}
	if (moves.size() == 0) {
		return;
	}
	std::random_shuffle(moves.begin(), moves.end());
	PossibleMove move = moves.at(0);
	for (PossibleMove m : moves) {
		if (m.material > move.material) {
			move = m;
			continue;
		}
		if (m.material == move.material) {
			// Pawn priority
			if (m.defensive && !move.defensive) {
				move = m;
				continue;
			}
		}
	}
	game.setSelectedPiece(move.from);
	game.setSelectedTarget(move.to);
	game.moveToTarget();
}