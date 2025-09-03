#include "stdio.h"
#include "defs.h"

static int IsPawnAttacking(const int sq, const int side, const S_BOARD *pos) {
	if(side == WHITE) {
		if(pos->pieces[sq-11] == WHITE_PAWN || pos->pieces[sq-9] == WHITE_PAWN) {
			return 1;
		}
	} else {
		if(pos->pieces[sq+11] == BLACK_PAWN || pos->pieces[sq+9] == BLACK_PAWN) {
			return 1;
		}
	}
	return 0;
}

static int IsKnightAttacking(const int sq, const int side, const S_BOARD *pos) {
	int pce;
	
	for(int i = 0; i < KN_DIR_COUNT; ++i) {
		pce = pos->pieces[sq + KnDir[i]];
		if(pce != OFFBOARD && IsKn(pce) && PieceCol[pce]==side) {
			return 1;
		}
	}
	return 0;
}

static int IsRookQueenAttacking(const int sq, const int side, const S_BOARD *pos) {
	int pce, t_sq, dir;
	
	for(int i = 0; i < RK_DIR_COUNT; ++i) {
		dir = RkDir[i];
		t_sq = sq + dir;
		pce = pos->pieces[t_sq];
		while(pce != OFFBOARD) {
			if(pce != EMPTY) {
				if(IsRQ(pce) && PieceCol[pce] == side) {
					return 1;
				}
				break;
			}
			t_sq += dir;
			pce = pos->pieces[t_sq];
		}
	}
	return 0;
}

static int IsBishopQueenAttacking(const int sq, const int side, const S_BOARD *pos) {
	int pce, t_sq, dir;
	
	for(int i = 0; i < BI_DIR_COUNT; ++i) {
		dir = BiDir[i];
		t_sq = sq + dir;
		pce = pos->pieces[t_sq];
		while(pce != OFFBOARD) {
			if(pce != EMPTY) {
				if(IsBQ(pce) && PieceCol[pce] == side) {
					return 1;
				}
				break;
			}
			t_sq += dir;
			pce = pos->pieces[t_sq];
		}
	}
	return 0;
}

static int IsKingAttacking(const int sq, const int side, const S_BOARD *pos) {
	int pce;
	
	for(int i = 0; i < KI_DIR_COUNT; ++i) {
		pce = pos->pieces[sq + KiDir[i]];
		if(pce != OFFBOARD && IsKi(pce) && PieceCol[pce]==side) {
			return 1;
		}
	}
	return 0;
}

int SqAttacked(const int sq, const int side, const S_BOARD *pos) {
	// Pawn attacks
	if(IsPawnAttacking(sq, side, pos)) {
		return 1;
	}
	
	// Knight attacks
	if(IsKnightAttacking(sq, side, pos)) {
		return 1;
	}
	
	// Rook and Queen attacks
	if(IsRookQueenAttacking(sq, side, pos)) {
		return 1;
	}
	
	// Bishop and Queen attacks
	if(IsBishopQueenAttacking(sq, side, pos)) {
		return 1;
	}
	
	// King attacks
	if(IsKingAttacking(sq, side, pos)) {
		return 1;
	}
	
	return 0;
}