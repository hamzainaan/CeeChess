#include "stdio.h"
#include "defs.h"
#include "config.h"

#define MOVE(f,t,ca,pro,fl) ( (f) | ((t) << 7) | ( (ca) << 14 ) | ( (pro) << 20 ) | (fl))
#define SQOFFBOARD(sq) (FilesBrd[(sq)]==OFFBOARD)

const int LoopSlidePce[8] = {
 WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, 0, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, 0
};

const int LoopNonSlidePce[6] = {
 WHITE_KNIGHT, WHITE_KING, 0, BLACK_KNIGHT, BLACK_KING, 0
};

const int LoopNonSlidePceMob[4] = {
 WHITE_KNIGHT, 0, BLACK_KNIGHT, 0
};

const int LoopSlideIndex[2] = { 0, 4 };
const int LoopNonSlideIndex[2] = { 0, 3 };
const int LoopNonSlideIndexMob[2] = { 0, 2 };

const int PceDir[13][8] = {
	{ 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0 },
	{ -8, -19,	-21, -12, 8, 19, 21, 12 },
	{ -9, -11, 11, 9, 0, 0, 0, 0 },
	{ -1, -10,	1, 10, 0, 0, 0, 0 },
	{ -1, -10,	1, 10, -9, -11, 11, 9 },
	{ -1, -10,	1, 10, -9, -11, 11, 9 },
	{ 0, 0, 0, 0, 0, 0, 0 },
	{ -8, -19,	-21, -12, 8, 19, 21, 12 },
	{ -9, -11, 11, 9, 0, 0, 0, 0 },
	{ -1, -10,	1, 10, 0, 0, 0, 0 },
	{ -1, -10,	1, 10, -9, -11, 11, 9 },
	{ -1, -10,	1, 10, -9, -11, 11, 9 }
};

const int NumDir[13] = {
 0, 0, 8, 4, 4, 8, 8, 0, 8, 4, 4, 8, 8
};

const int VictimScore[13] = { 0, 100, 200, 300, 400, 500, 600, 100, 200, 300, 400, 500, 600 };
static int MvvLvaScores[13][13];

void InitMvvLva() {
	int Attacker;
	int Victim;
	for(Attacker = WHITE_PAWN; Attacker <= BLACK_KING; ++Attacker) {
		for(Victim = WHITE_PAWN; Victim <= BLACK_KING; ++Victim) {
			MvvLvaScores[Victim][Attacker] = VictimScore[Victim] + 6 - ( VictimScore[Attacker] / 100);
		}
	}
}

int MoveExists(S_BOARD *pos, const int move) {

	S_MOVELIST list[1];
    GenerateAllMoves(pos,list);

    int MoveNum = 0;
	for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {

        if ( !MakeMove(pos,list->moves[MoveNum].move))  {
            continue;
        }
        TakeMove(pos);
		if(list->moves[MoveNum].move == move) {
			return 1;
		}
    }
	return 0;
}

static void AddQuietMove( const S_BOARD *pos, int move, S_MOVELIST *list ) {

	list->moves[list->count].move = move;

  	if(pos->searchKillers[0][pos->ply] == move) {
		list->moves[list->count].score = 900000;
	} else if (pos->searchKillers[1][pos->ply] == move) {
		list->moves[list->count].score = 800000;
	} else if (pos->searchKillers[0][pos->ply - 2] == move) {
		list->moves[list->count].score = 700000;
	} else if (pos->searchKillers[1][pos->ply - 2] == move) {
		list->moves[list->count].score = 600000;
	}  else {
		list->moves[list->count].score = MIN(pos->searchHistory[pos->pieces[FROMSQ(move)]][TOSQ(move)], 500000);
	}
	list->count++;
}

static void AddCaptureMove( const S_BOARD *pos, int move, S_MOVELIST *list ) {

	list->moves[list->count].move = move;
	
	// Get captured piece and attacker
	int captured = CAPTURED(move);
	int attacker = pos->pieces[FROMSQ(move)];
	
	// Calculate MVV/LVA score
	int mvvLvaScore = MvvLvaScores[captured][attacker];
	
	if (attacker == WHITE_QUEEN || attacker == BLACK_QUEEN) {
		int seeScore = SEE(pos, move);
		
		if (seeScore < 0) {
			list->moves[list->count].score = seeScore;
		} else {
			list->moves[list->count].score = mvvLvaScore + 1000000;
		}
	} else {
		// Only use SEE for captures that might be bad (when attacker value >= captured value)
		if (PieceValMG[attacker] >= PieceValMG[captured]) {
			// Use Static Exchange Evaluation for move ordering
			int seeScore = SEE(pos, move);
			
			// If SEE score is positive, prioritize it highly
			if (seeScore > 0) {
				list->moves[list->count].score = seeScore + 1000000;
			} else {
				// If SEE score is negative, use MVV/LVA but with lower priority
				list->moves[list->count].score = mvvLvaScore;
			}
		} else {
			// For obviously good captures (capturing higher value piece), just use MVV/LVA + bonus
			list->moves[list->count].score = mvvLvaScore + 1000000;
		}
	}
	
	list->count++;
}

static void AddEnPassantMove( const S_BOARD *pos, int move, S_MOVELIST *list ) {

	list->moves[list->count].move = move;
	list->moves[list->count].score = 105 + 1000000;
	list->count++;
}

static void AddWhitePawnCapMove( const S_BOARD *pos, const int from, const int to, const int cap, S_MOVELIST *list ) {

	if(RanksBrd[from] == RANK_7) {
		AddCaptureMove(pos, MOVE(from,to,cap,WHITE_QUEEN,0), list);
		AddCaptureMove(pos, MOVE(from,to,cap,WHITE_ROOK,0), list);
		AddCaptureMove(pos, MOVE(from,to,cap,WHITE_BISHOP,0), list);
		AddCaptureMove(pos, MOVE(from,to,cap,WHITE_KNIGHT,0), list);
	} else {
		AddCaptureMove(pos, MOVE(from,to,cap,EMPTY,0), list);
	}
}

static void AddWhitePawnMove( const S_BOARD *pos, const int from, const int to, S_MOVELIST *list ) {

	if(RanksBrd[from] == RANK_7) {
		AddQuietMove(pos, MOVE(from,to,EMPTY,WHITE_QUEEN,0), list);
		AddQuietMove(pos, MOVE(from,to,EMPTY,WHITE_ROOK,0), list);
		AddQuietMove(pos, MOVE(from,to,EMPTY,WHITE_BISHOP,0), list);
		AddQuietMove(pos, MOVE(from,to,EMPTY,WHITE_KNIGHT,0), list);
	} else {
		AddQuietMove(pos, MOVE(from,to,EMPTY,EMPTY,0), list);
	}
}

static void AddBlackPawnCapMove( const S_BOARD *pos, const int from, const int to, const int cap, S_MOVELIST *list ) {

	if(RanksBrd[from] == RANK_2) {
		AddCaptureMove(pos, MOVE(from,to,cap,BLACK_QUEEN,0), list);
		AddCaptureMove(pos, MOVE(from,to,cap,BLACK_ROOK,0), list);
		AddCaptureMove(pos, MOVE(from,to,cap,BLACK_BISHOP,0), list);
		AddCaptureMove(pos, MOVE(from,to,cap,BLACK_KNIGHT,0), list);
	} else {
		AddCaptureMove(pos, MOVE(from,to,cap,EMPTY,0), list);
	}
}

static void AddBlackPawnMove( const S_BOARD *pos, const int from, const int to, S_MOVELIST *list ) {

	if(RanksBrd[from] == RANK_2) {
		AddQuietMove(pos, MOVE(from,to,EMPTY,BLACK_QUEEN,0), list);
		AddQuietMove(pos, MOVE(from,to,EMPTY,BLACK_ROOK,0), list);
		AddQuietMove(pos, MOVE(from,to,EMPTY,BLACK_BISHOP,0), list);
		AddQuietMove(pos, MOVE(from,to,EMPTY,BLACK_KNIGHT,0), list);
	} else {
		AddQuietMove(pos, MOVE(from,to,EMPTY,EMPTY,0), list);
	}
}

void GenerateAllMoves(const S_BOARD *pos, S_MOVELIST *list) {

	list->count = 0;

	int pce = EMPTY;
	int side = pos->side;
	int sq = 0; int t_sq = 0;
	int pceNum = 0;
	int dir = 0;
	int index = 0;
	int pceIndex = 0;

	if(side == WHITE) {

		for(pceNum = 0; pceNum < pos->pceNum[WHITE_PAWN]; ++pceNum) {
			sq = pos->pList[WHITE_PAWN][pceNum];

			if(pos->pieces[sq + 10] == EMPTY) {
				AddWhitePawnMove(pos, sq, sq+10, list);
				if(RanksBrd[sq] == RANK_2 && pos->pieces[sq + 20] == EMPTY) {
					AddQuietMove(pos, MOVE(sq,(sq+20),EMPTY,EMPTY,0x80000),list);
				}
			}

			if(!SQOFFBOARD(sq + 9) && PieceCol[pos->pieces[sq + 9]] == BLACK) {
				AddWhitePawnCapMove(pos, sq, sq+9, pos->pieces[sq + 9], list);
			}
			if(!SQOFFBOARD(sq + 11) && PieceCol[pos->pieces[sq + 11]] == BLACK) {
				AddWhitePawnCapMove(pos, sq, sq+11, pos->pieces[sq + 11], list);
			}

			if(pos->enPas != NO_SQ) {
				if(sq + 9 == pos->enPas) {
					AddEnPassantMove(pos, MOVE(sq,sq + 9,EMPTY,EMPTY,0x40000), list);
				}
				if(sq + 11 == pos->enPas) {
					AddEnPassantMove(pos, MOVE(sq,sq + 11,EMPTY,EMPTY,0x40000), list);
				}
			}
		}

		if(pos->castlePerm & WKCA) {
			if(pos->pieces[F1] == EMPTY && pos->pieces[G1] == EMPTY) {
				if(!SqAttacked(E1,BLACK,pos) && !SqAttacked(F1,BLACK,pos) ) {
					AddQuietMove(pos, MOVE(E1, G1, EMPTY, EMPTY, 0x1000000), list);
				}
			}
		}

		if(pos->castlePerm & WQCA) {
			if(pos->pieces[D1] == EMPTY && pos->pieces[C1] == EMPTY && pos->pieces[B1] == EMPTY) {
				if(!SqAttacked(E1,BLACK,pos) && !SqAttacked(D1,BLACK,pos) ) {
					AddQuietMove(pos, MOVE(E1, C1, EMPTY, EMPTY, 0x1000000), list);
				}
			}
		}

	} else {

		for(pceNum = 0; pceNum < pos->pceNum[BLACK_PAWN]; ++pceNum) {
			sq = pos->pList[BLACK_PAWN][pceNum];

			if(pos->pieces[sq - 10] == EMPTY) {
				AddBlackPawnMove(pos, sq, sq-10, list);
				if(RanksBrd[sq] == RANK_7 && pos->pieces[sq - 20] == EMPTY) {
					AddQuietMove(pos, MOVE(sq,(sq-20),EMPTY,EMPTY,0x80000),list);
				}
			}

			if(!SQOFFBOARD(sq - 9) && PieceCol[pos->pieces[sq - 9]] == WHITE) {
				AddBlackPawnCapMove(pos, sq, sq-9, pos->pieces[sq - 9], list);
			}

			if(!SQOFFBOARD(sq - 11) && PieceCol[pos->pieces[sq - 11]] == WHITE) {
				AddBlackPawnCapMove(pos, sq, sq-11, pos->pieces[sq - 11], list);
			}
			if(pos->enPas != NO_SQ) {
				if(sq - 9 == pos->enPas) {
					AddEnPassantMove(pos, MOVE(sq,sq - 9,EMPTY,EMPTY,0x40000), list);
				}
				if(sq - 11 == pos->enPas) {
					AddEnPassantMove(pos, MOVE(sq,sq - 11,EMPTY,EMPTY,0x40000), list);
				}
			}
		}

		// castling
		if(pos->castlePerm &  BKCA) {
			if(pos->pieces[F8] == EMPTY && pos->pieces[G8] == EMPTY) {
				if(!SqAttacked(E8,WHITE,pos) && !SqAttacked(F8,WHITE,pos) ) {
					AddQuietMove(pos, MOVE(E8, G8, EMPTY, EMPTY, 0x1000000), list);
				}
			}
		}

		if(pos->castlePerm &  BQCA) {
			if(pos->pieces[D8] == EMPTY && pos->pieces[C8] == EMPTY && pos->pieces[B8] == EMPTY) {
				if(!SqAttacked(E8,WHITE,pos) && !SqAttacked(D8,WHITE,pos) ) {
					AddQuietMove(pos, MOVE(E8, C8, EMPTY, EMPTY, 0x1000000), list);
				}
			}
		}
	}

	/* Loop for slide pieces */
	pceIndex = LoopSlideIndex[side];
	pce = LoopSlidePce[pceIndex++];
	while( pce != 0) {

		for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
			sq = pos->pList[pce][pceNum];

			for(index = 0; index < NumDir[pce]; ++index) {
				dir = PceDir[pce][index];
				t_sq = sq + dir;

				while(!SQOFFBOARD(t_sq)) {
					if(pos->pieces[t_sq] != EMPTY) {
						if( PieceCol[pos->pieces[t_sq]] == (side ^ 1)) {
							AddCaptureMove(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list);
						}
						break;
					}
					AddQuietMove(pos, MOVE(sq, t_sq, EMPTY, EMPTY, 0), list);
					t_sq += dir;
				}
			}
		}

		pce = LoopSlidePce[pceIndex++];
	}

	/* Loop for non slide */
	pceIndex = LoopNonSlideIndex[side];
	pce = LoopNonSlidePce[pceIndex++];

	while( pce != 0) {

		for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
			sq = pos->pList[pce][pceNum];

			for(index = 0; index < NumDir[pce]; ++index) {
				dir = PceDir[pce][index];
				t_sq = sq + dir;

				if(SQOFFBOARD(t_sq)) {
					continue;
				}

				if(pos->pieces[t_sq] != EMPTY) {
					if( PieceCol[pos->pieces[t_sq]] == (side ^ 1)) {
						AddCaptureMove(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list);
					}
					continue;
				}
				AddQuietMove(pos, MOVE(sq, t_sq, EMPTY, EMPTY, 0), list);
			}
		}

		pce = LoopNonSlidePce[pceIndex++];
	}
}

void GenerateAllCaps(const S_BOARD *pos, S_MOVELIST *list) {

	list->count = 0;

	int pce = EMPTY;
	int side = pos->side;
	int sq = 0; int t_sq = 0;
	int pceNum = 0;
	int dir = 0;
	int index = 0;
	int pceIndex = 0;

	if(side == WHITE) {

		for(pceNum = 0; pceNum < pos->pceNum[WHITE_PAWN]; ++pceNum) {
			sq = pos->pList[WHITE_PAWN][pceNum];

			if(!SQOFFBOARD(sq + 9) && PieceCol[pos->pieces[sq + 9]] == BLACK) {
				AddWhitePawnCapMove(pos, sq, sq+9, pos->pieces[sq + 9], list);
			}
			if(!SQOFFBOARD(sq + 11) && PieceCol[pos->pieces[sq + 11]] == BLACK) {
				AddWhitePawnCapMove(pos, sq, sq+11, pos->pieces[sq + 11], list);
			}

			if(pos->enPas != NO_SQ) {
				if(sq + 9 == pos->enPas) {
					AddEnPassantMove(pos, MOVE(sq,sq + 9,EMPTY,EMPTY,0x40000), list);
				}
				if(sq + 11 == pos->enPas) {
					AddEnPassantMove(pos, MOVE(sq,sq + 11,EMPTY,EMPTY,0x40000), list);
				}
			}
		}

	} else {

		for(pceNum = 0; pceNum < pos->pceNum[BLACK_PAWN]; ++pceNum) {
			sq = pos->pList[BLACK_PAWN][pceNum];

			if(!SQOFFBOARD(sq - 9) && PieceCol[pos->pieces[sq - 9]] == WHITE) {
				AddBlackPawnCapMove(pos, sq, sq-9, pos->pieces[sq - 9], list);
			}

			if(!SQOFFBOARD(sq - 11) && PieceCol[pos->pieces[sq - 11]] == WHITE) {
				AddBlackPawnCapMove(pos, sq, sq-11, pos->pieces[sq - 11], list);
			}
			if(pos->enPas != NO_SQ) {
				if(sq - 9 == pos->enPas) {
					AddEnPassantMove(pos, MOVE(sq,sq - 9,EMPTY,EMPTY,0x40000), list);
				}
				if(sq - 11 == pos->enPas) {
					AddEnPassantMove(pos, MOVE(sq,sq - 11,EMPTY,EMPTY,0x40000), list);
				}
			}
		}
	}

	/* Loop for slide pieces */
	pceIndex = LoopSlideIndex[side];
	pce = LoopSlidePce[pceIndex++];
	while( pce != 0) {

		for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
			sq = pos->pList[pce][pceNum];

			for(index = 0; index < NumDir[pce]; ++index) {
				dir = PceDir[pce][index];
				t_sq = sq + dir;

				while(!SQOFFBOARD(t_sq)) {
					if(pos->pieces[t_sq] != EMPTY) {
						if( PieceCol[pos->pieces[t_sq]] == (side ^ 1)) {
							AddCaptureMove(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list);
						}
						break;
					}
					t_sq += dir;
				}
			}
		}

		pce = LoopSlidePce[pceIndex++];
	}

	/* Loop for non slide */
	pceIndex = LoopNonSlideIndex[side];
	pce = LoopNonSlidePce[pceIndex++];

	while( pce != 0) {

		for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
			sq = pos->pList[pce][pceNum];

			for(index = 0; index < NumDir[pce]; ++index) {
				dir = PceDir[pce][index];
				t_sq = sq + dir;

				if(SQOFFBOARD(t_sq)) {
					continue;
				}

				if(pos->pieces[t_sq] != EMPTY) {
					if( PieceCol[pos->pieces[t_sq]] == (side ^ 1)) {
						AddCaptureMove(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list);
					}
					continue;
				}
			}
		}

		pce = LoopNonSlidePce[pceIndex++];
	}
}

inline int GetMobility(const S_BOARD *pos, int side) {

	int pce = EMPTY;
	int sq = 0; int t_sq = 0;
	int pceNum = 0;
	int dir = 0;
	int index = 0;
	int pceIndex = 0;
	int count = 0;

	if(side == WHITE) {
		for(pceNum = 0; pceNum < pos->pceNum[WHITE_PAWN]; ++pceNum) {
			sq = pos->pList[WHITE_PAWN][pceNum];

			if(pos->pieces[sq + 10] == EMPTY) {
				count++;
				if(RanksBrd[sq] == RANK_2 && pos->pieces[sq + 20] == EMPTY) {
					count++;
				}
			}

			if(!SQOFFBOARD(sq + 9) && PieceCol[pos->pieces[sq + 9]] == BLACK) {
				count++;
			}
			if(!SQOFFBOARD(sq + 11) && PieceCol[pos->pieces[sq + 11]] == BLACK) {
				count++;
			}

			if(pos->enPas != NO_SQ) {
				if(sq + 9 == pos->enPas) {
					count++;
				}
				if(sq + 11 == pos->enPas) {
					count++;
				}
			}
		}
	} else {
		for(pceNum = 0; pceNum < pos->pceNum[BLACK_PAWN]; ++pceNum) {
			sq = pos->pList[BLACK_PAWN][pceNum];

			if(pos->pieces[sq - 10] == EMPTY) {
				count++;
				if(RanksBrd[sq] == RANK_7 && pos->pieces[sq - 20] == EMPTY) {
					count++;
				}
			}

			if(!SQOFFBOARD(sq - 9) && PieceCol[pos->pieces[sq - 9]] == WHITE) {
				count++;
			}

			if(!SQOFFBOARD(sq - 11) && PieceCol[pos->pieces[sq - 11]] == WHITE) {
				count++;
			}
			if(pos->enPas != NO_SQ) {
				if(sq - 9 == pos->enPas) {
					count++;
				}
				if(sq - 11 == pos->enPas) {
					count++;
				}
			}
		}
	}

	/* Loop for slide pieces */
	pceIndex = LoopSlideIndex[side];
	pce = LoopSlidePce[pceIndex++];
	while( pce != 0) {

		for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
			sq = pos->pList[pce][pceNum];

			for(index = 0; index < NumDir[pce]; ++index) {
				dir = PceDir[pce][index];
				t_sq = sq + dir;

				while(!SQOFFBOARD(t_sq)) {
					if(pos->pieces[t_sq] != EMPTY) {
						count += (PieceCol[pos->pieces[t_sq]] == (side ^ 1));
						break;
					}
					count++;
					t_sq += dir;
				}
			}
		}

		pce = LoopSlidePce[pceIndex++];
	}

	/* Loop for non slide */
	// not using king moves in mobility
	pceIndex = LoopNonSlideIndexMob[side];
	pce = LoopNonSlidePceMob[pceIndex++];

	while( pce != 0) {

		for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
			sq = pos->pList[pce][pceNum];

			for(index = 0; index < NumDir[pce]; ++index) {
				dir = PceDir[pce][index];
				t_sq = sq + dir;

				if(SQOFFBOARD(t_sq)) {
					continue;
				}

				if(pos->pieces[t_sq] != EMPTY) {
					count += (PieceCol[pos->pieces[t_sq]] == (side ^ 1));
					continue;
				}
				count++;
			}
		}

		pce = LoopNonSlidePce[pceIndex++];
	}
	return count;
}