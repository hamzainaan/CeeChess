#include "stdio.h"
#include "defs.h"

#include "string.h"

// Static Exchange Evaluation function - optimized version
int SEE(const S_BOARD *pos, const int move) {
    int from = FROMSQ(move);
    int to = TOSQ(move);
    int captured = CAPTURED(move);
    int attacker = pos->pieces[from];
    
    // Initial gain is the value of the captured piece
    int gain[32] = {0};
    int depth = 0;
    
    // If no capture, return 0 (quick exit for most moves)
    if (captured == EMPTY) {
        // Handle en passant captures
        if ((move & 0x40000) != 0) { // En passant flag
            gain[depth] = PieceValMG[WHITE_PAWN]; // Pawn value
        } else {
            return 0;
        }
    } else {
        gain[depth] = PieceValMG[captured];
    }
    
    // Use a local array instead of copying the entire board structure
    int pieces[120];
    memcpy(pieces, pos->pieces, 120 * sizeof(int));
    
    // Remove the initial attacker from the board
    pieces[from] = EMPTY;
    
    // Make the initial capture
    int side = 1 - PieceCol[attacker];
    
    // Find the least valuable attacker
    while (1) {
        depth++;
        if (depth >= 32) break; // Safety check
        
        // Find the least valuable attacker for the side to move
        int attackerPce = EMPTY;
        int attackerVal = 10000; // Higher than any piece value
        int attackerSq = -1;
        
        // Check pawn attacks
        if (side == WHITE) {
            // White pawns attack down-left and down-right
            if (to >= 21 && to <= 98) {
                if (pieces[to-11] == WHITE_PAWN) {
                    int newAttackerVal = PieceValMG[WHITE_PAWN];
                    if (newAttackerVal < attackerVal) {
                        attackerPce = WHITE_PAWN;
                        attackerVal = newAttackerVal;
                        attackerSq = to-11;
                    }
                }
                if (pieces[to-9] == WHITE_PAWN) {
                    int newAttackerVal = PieceValMG[WHITE_PAWN];
                    if (newAttackerVal < attackerVal) {
                        attackerPce = WHITE_PAWN;
                        attackerVal = newAttackerVal;
                        attackerSq = to-9;
                    }
                }
            }
        } else {
            // Black pawns attack up-left and up-right
            if (to >= 21 && to <= 98) {
                if (pieces[to+11] == BLACK_PAWN) {
                    int newAttackerVal = PieceValMG[BLACK_PAWN];
                    if (newAttackerVal < attackerVal) {
                        attackerPce = BLACK_PAWN;
                        attackerVal = newAttackerVal;
                        attackerSq = to+11;
                    }
                }
                if (pieces[to+9] == BLACK_PAWN) {
                    int newAttackerVal = PieceValMG[BLACK_PAWN];
                    if (newAttackerVal < attackerVal) {
                        attackerPce = BLACK_PAWN;
                        attackerVal = newAttackerVal;
                        attackerSq = to+9;
                    }
                }
            }
        }
        
        // Check knight attacks
        for (int i = 0; i < 8; ++i) {
            int t_sq = to + KnDir[i];
            int pce = pieces[t_sq];
            if (pce != OFFBOARD && IsKn(pce) && PieceCol[pce] == side) {
                int newAttackerVal = PieceValMG[pce];
                if (newAttackerVal < attackerVal) {
                    attackerPce = pce;
                    attackerVal = newAttackerVal;
                    attackerSq = t_sq;
                }
            }
        }
        
        // Check bishop and queen attacks
        for (int i = 0; i < 4; ++i) {
            int dir = BiDir[i];
            int t_sq = to + dir;
            int pce = pieces[t_sq];
            while (pce != OFFBOARD) {
                if (pce != EMPTY) {
                    if (IsBQ(pce) && PieceCol[pce] == side) {
                        int newAttackerVal = PieceValMG[pce];
                        if (newAttackerVal < attackerVal) {
                            attackerPce = pce;
                            attackerVal = newAttackerVal;
                            attackerSq = t_sq;
                        }
                    }
                    break;
                }
                t_sq += dir;
                pce = pieces[t_sq];
            }
        }
        
        // Check rook and queen attacks
        for (int i = 0; i < 4; ++i) {
            int dir = RkDir[i];
            int t_sq = to + dir;
            int pce = pieces[t_sq];
            while (pce != OFFBOARD) {
                if (pce != EMPTY) {
                    if (IsRQ(pce) && PieceCol[pce] == side) {
                        int newAttackerVal = PieceValMG[pce];
                        if (newAttackerVal < attackerVal) {
                            attackerPce = pce;
                            attackerVal = newAttackerVal;
                            attackerSq = t_sq;
                        }
                    }
                    break;
                }
                t_sq += dir;
                pce = pieces[t_sq];
            }
        }
        
        // Check king attacks
        for (int i = 0; i < 8; ++i) {
            int t_sq = to + KiDir[i];
            int pce = pieces[t_sq];
            if (pce != OFFBOARD && IsKi(pce) && PieceCol[pce] == side) {
                int newAttackerVal = PieceValMG[pce];
                if (newAttackerVal < attackerVal) {
                    attackerPce = pce;
                    attackerVal = newAttackerVal;
                    attackerSq = t_sq;
                }
            }
        }
        
        // No more attackers
        if (attackerPce == EMPTY) break;
        
        // Make the capture
        gain[depth] = PieceValMG[attacker] - gain[depth-1];
        if (gain[depth] < 0) break; // Attacker loses material
        
        // Update the board for the next capture
        pieces[attackerSq] = EMPTY;
        attacker = attackerPce;
        side = 1 - side;
    }
    
    // Unwind the captures
    while (--depth) {
        gain[depth-1] = -MAX(-gain[depth-1], gain[depth]);
    }
    
    return gain[0];
}

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