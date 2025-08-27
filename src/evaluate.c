#include "stdio.h"
#include "defs.h"
#include "eval.h"

int DistTable[64][64];

void InitEval() {
	for (int i = 0; i < 64; ++i) {
		for (int j = 0; j < 64; ++j) {
			DistTable[i][j] = 14 - ( abs ( COL(i) - COL(j) ) + abs ( ROW(i) - ROW(j) ) );
      	}
   	}
}

static inline int MaterialDraw(const S_BOARD *pos) {

    if (!pos->pceNum[WHITE_ROOK] && !pos->pceNum[BLACK_ROOK] && !pos->pceNum[WHITE_QUEEN] && !pos->pceNum[BLACK_QUEEN]) {
	  if (!pos->pceNum[BLACK_BISHOP] && !pos->pceNum[WHITE_BISHOP]) {
	      if (pos->pceNum[WHITE_KNIGHT] < 3 && pos->pceNum[BLACK_KNIGHT] < 3) {  return 1; }
	  } else if (!pos->pceNum[WHITE_KNIGHT] && !pos->pceNum[BLACK_KNIGHT]) {
	     if (abs(pos->pceNum[WHITE_BISHOP] - pos->pceNum[BLACK_BISHOP]) < 2) { return 1; }
	  } else if ((pos->pceNum[WHITE_KNIGHT] < 3 && !pos->pceNum[WHITE_BISHOP]) || (pos->pceNum[WHITE_BISHOP] == 1 && !pos->pceNum[WHITE_KNIGHT])) {
	    if ((pos->pceNum[BLACK_KNIGHT] < 3 && !pos->pceNum[BLACK_BISHOP]) || (pos->pceNum[BLACK_BISHOP] == 1 && !pos->pceNum[BLACK_KNIGHT]))  { return 1; }
	  }
	} else if (!pos->pceNum[WHITE_QUEEN] && !pos->pceNum[BLACK_QUEEN]) {
        if (pos->pceNum[WHITE_ROOK] == 1 && pos->pceNum[BLACK_ROOK] == 1) {
            if ((pos->pceNum[WHITE_KNIGHT] + pos->pceNum[WHITE_BISHOP]) < 2 && (pos->pceNum[BLACK_KNIGHT] + pos->pceNum[BLACK_BISHOP]) < 2)	{ return 1; }
        } else if (pos->pceNum[WHITE_ROOK] == 1 && !pos->pceNum[BLACK_ROOK]) {
            if ((pos->pceNum[WHITE_KNIGHT] + pos->pceNum[WHITE_BISHOP] == 0) && (((pos->pceNum[BLACK_KNIGHT] + pos->pceNum[BLACK_BISHOP]) == 1) || ((pos->pceNum[BLACK_KNIGHT] + pos->pceNum[BLACK_BISHOP]) == 2))) { return 1; }
        } else if (pos->pceNum[BLACK_ROOK] == 1 && !pos->pceNum[WHITE_ROOK]) {
            if ((pos->pceNum[BLACK_KNIGHT] + pos->pceNum[BLACK_BISHOP] == 0) && (((pos->pceNum[WHITE_KNIGHT] + pos->pceNum[WHITE_BISHOP]) == 1) || ((pos->pceNum[WHITE_KNIGHT] + pos->pceNum[WHITE_BISHOP]) == 2))) { return 1; }
        }
    }
  return 0;
}

int EvalPosition(S_BOARD *pos) {;

	// test for drawn position before doing anything
	if((!pos->pceNum[WHITE_PAWN] && !pos->pceNum[BLACK_PAWN] && MaterialDraw(pos) == 1)) {
		return 0;
	}

	int diagonal_bonus;
	int pce;
	int pceNum;
	int sq;
	int phase = totalPhase;
	int wPhase = 0;
	int bPhase = 0;
	//int mobility = GetMobility(pos, WHITE) - GetMobility(pos, BLACK);
	int scoreMG = (pos->material[WHITE] - pos->material[BLACK]); //+ (mobilityFactorMG * mobility);
	int scoreEG = (pos->material[WHITE + 2] - pos->material[BLACK + 2]); //+ (mobilityFactorEG * mobility);

	int kingScoreW = 0;
	int kingScoreB = 0;

	// get king squares to calculate king tropism
	pce = WHITE_KING;
	sq = pos->pList[pce][0];
	int wKsq64 = SQ64(sq);

	scoreMG += KingMG[SQ64(sq)];
	scoreEG += KingEG[SQ64(sq)];

	// if there are semi-open files near this king, boost attack score for enemy
	for (int i_sq = sq - 8; i_sq <= sq + 8; i_sq += 8) {
		if (FilesBrd[i_sq] == OFFBOARD)
			continue;
		kingScoreB -= KingSemiOpen * !(pos->pawns[WHITE] & FileBBMask[FilesBrd[i_sq]]);
	}

	pce = BLACK_KING;
	sq = pos->pList[pce][0];
	int bKsq64 = SQ64(sq);

	scoreMG -= KingMG[MIRROR64(SQ64(sq))];
	scoreEG -= KingEG[MIRROR64(SQ64(sq))];
	
	// if there are semi-open files near this king, boost attack score for enemy
	for (int i_sq = sq - 8; i_sq <= sq + 8; i_sq += 8) {
		if (FilesBrd[i_sq] == OFFBOARD)
			continue;
		kingScoreW += KingSemiOpen * !(pos->pawns[BLACK] & FileBBMask[FilesBrd[i_sq]]);
	}
	
	pce = WHITE_PAWN;
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];

		int passed = 0;
		int connected = 0;

		scoreMG += PawnMG[SQ64(sq)];
		scoreEG += PawnEG[SQ64(sq)];

		if( (IsolatedMask[SQ64(sq)] & pos->pawns[WHITE]) == 0) {
			//printf("WHITE_PAWN Iso:%s\n",PrSq(sq));
			scoreMG += PawnIsolatedMG;
			scoreEG += PawnIsolatedEG;
		}

		if( (WhitePassedMask[SQ64(sq)] & pos->pawns[BLACK]) == 0) {
			//printf("WHITE_PAWN Passed:%s\n",PrSq(sq));
			passed = 1;
			scoreMG += PawnPassedMG[RanksBrd[sq]];
			scoreEG += PawnPassedEG[RanksBrd[sq]];
		}

		if ((WhiteConnectedMask[SQ64(sq)] & pos->pawns[WHITE]) != 0) {
			connected = 1;
			scoreMG += PawnConnectedMG;
			scoreEG += PawnConnectedEG;
		}

		if (passed && connected) {
			scoreMG += PawnPassedConnectedMG[RanksBrd[sq]];
			scoreEG += PawnPassedConnectedEG[RanksBrd[sq]];
		}
	}

	pce = BLACK_PAWN;
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];

		int passed = 0;
		int connected = 0;

		scoreMG -= PawnMG[MIRROR64(SQ64(sq))];
		scoreEG -= PawnEG[MIRROR64(SQ64(sq))];

		if( (IsolatedMask[SQ64(sq)] & pos->pawns[BLACK]) == 0) {
			//printf("BLACK_PAWN Iso:%s\n",PrSq(sq));
			scoreMG -= PawnIsolatedMG;
			scoreEG -= PawnIsolatedEG;
		}

		if( (BlackPassedMask[SQ64(sq)] & pos->pawns[WHITE]) == 0) {
			//printf("BLACK_PAWN Passed:%s\n",PrSq(sq));
			passed = 1;
			scoreMG -= PawnPassedMG[7 - RanksBrd[sq]];
			scoreEG -= PawnPassedEG[7 - RanksBrd[sq]];
		}

		if ((BlackConnectedMask[SQ64(sq)] & pos->pawns[BLACK]) != 0) {
			connected = 1;
			scoreMG -= PawnConnectedMG;
			scoreEG -= PawnConnectedEG;
		}

		if (passed && connected) {
			scoreMG -= PawnPassedConnectedMG[7 - RanksBrd[sq]];
			scoreEG -= PawnPassedConnectedEG[7 - RanksBrd[sq]];
		}
	}

	pce = WHITE_KNIGHT;
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];

		scoreMG += KnightMG[SQ64(sq)];
		scoreEG += KnightEG[SQ64(sq)];
		phase -= minorPhase;

		wPhase += minorPhase;
		kingScoreW += (TropismValues[0] * DistTable[SQ64(sq)][bKsq64]) / 16;
	}

	pce = BLACK_KNIGHT;
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];
		scoreMG -= KnightMG[MIRROR64(SQ64(sq))];
		scoreEG -= KnightEG[MIRROR64(SQ64(sq))];
		phase -= minorPhase;
		bPhase += minorPhase;
		kingScoreB -= (TropismValues[0] * DistTable[SQ64(sq)][wKsq64]) / 16;
	}

	pce = WHITE_BISHOP;
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];

		scoreMG += BishopMG[SQ64(sq)];
		scoreEG += BishopEG[SQ64(sq)];
		phase -= minorPhase;

		wPhase += minorPhase;
		diagonal_bonus = bonus_dia_distance[abs(diag_ne[SQ64(sq)] - diag_ne[bKsq64])] + bonus_dia_distance[abs(diag_nw[SQ64(sq)] - diag_nw[bKsq64])];
		kingScoreW += (TropismValues[1] * (DistTable[SQ64(sq)][bKsq64] + diagonal_bonus)) / 16;
	}

	pce = BLACK_BISHOP;
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];
		scoreMG -= BishopMG[MIRROR64(SQ64(sq))];
		scoreEG -= BishopEG[MIRROR64(SQ64(sq))];
		phase -= minorPhase;
		bPhase += minorPhase;
		diagonal_bonus = bonus_dia_distance[abs(diag_ne[SQ64(sq)] - diag_ne[wKsq64])] + bonus_dia_distance[abs(diag_nw[SQ64(sq)] - diag_nw[wKsq64])];
		kingScoreB -= (TropismValues[1] * (DistTable[SQ64(sq)][wKsq64] + diagonal_bonus)) / 16;
	}

	pce = WHITE_ROOK;
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];
		scoreMG += RookMG[SQ64(sq)];
		scoreEG += RookEG[SQ64(sq)];

		if(!(pos->pawns[BOTH] & FileBBMask[FilesBrd[sq]])) {
			scoreMG += RookOpenFileMG;
		} else if(!(pos->pawns[WHITE] & FileBBMask[FilesBrd[sq]])) {
			scoreMG += RookSemiOpenFileMG;
		}
		phase -= rookPhase;
		wPhase += rookPhase;
		kingScoreW += (TropismValues[2] * DistTable[SQ64(sq)][bKsq64]) / 16;
	}

	pce = BLACK_ROOK;
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];
		scoreMG -= RookMG[MIRROR64(SQ64(sq))];
		scoreEG -= RookEG[MIRROR64(SQ64(sq))];

		if(!(pos->pawns[BOTH] & FileBBMask[FilesBrd[sq]])) {
			scoreMG -= RookOpenFileMG;
		} else if(!(pos->pawns[BLACK] & FileBBMask[FilesBrd[sq]])) {
			scoreMG -= RookSemiOpenFileMG;
		}
		phase -= rookPhase;
		bPhase += rookPhase;
		kingScoreB -= (TropismValues[2] * DistTable[SQ64(sq)][wKsq64]) / 16;
	}

	pce = WHITE_QUEEN;
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];

		scoreMG += QueenMG[SQ64(sq)];
		scoreEG += QueenEG[SQ64(sq)];

		if(!(pos->pawns[BOTH] & FileBBMask[FilesBrd[sq]])) {
			scoreMG += QueenOpenFileMG;
		} else if(!(pos->pawns[WHITE] & FileBBMask[FilesBrd[sq]])) {
			scoreMG += QueenSemiOpenFileMG;
		}
		phase -= queenPhase;
		wPhase += queenPhase;
		diagonal_bonus = bonus_dia_distance[abs(diag_ne[SQ64(sq)] - diag_ne[bKsq64])] + bonus_dia_distance[abs(diag_nw[SQ64(sq)] - diag_nw[bKsq64])];
		kingScoreW += (TropismValues[3] * (DistTable[SQ64(sq)][bKsq64] + diagonal_bonus)) / 16;
	}

	pce = BLACK_QUEEN;
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];
		scoreMG -= QueenMG[MIRROR64(SQ64(sq))];
		scoreEG -= QueenEG[MIRROR64(SQ64(sq))];

		if(!(pos->pawns[BOTH] & FileBBMask[FilesBrd[sq]])) {
			scoreMG -= QueenOpenFileMG;
		} else if(!(pos->pawns[BLACK] & FileBBMask[FilesBrd[sq]])) {
			scoreMG -= QueenSemiOpenFileMG;
		}
		phase -= queenPhase;
		bPhase += queenPhase;
		diagonal_bonus = bonus_dia_distance[abs(diag_ne[SQ64(sq)] - diag_ne[wKsq64])] + bonus_dia_distance[abs(diag_nw[SQ64(sq)] - diag_nw[wKsq64])];
		kingScoreB -= (TropismValues[3] * (DistTable[SQ64(sq)][wKsq64] + diagonal_bonus)) / 16;
	}
	
	pce = WHITE_KING;
	sq = pos->pList[pce][0];
	scoreMG += KingMG[SQ64(sq)];
	scoreEG += KingEG[SQ64(sq)];
	pce = BLACK_KING;
	sq = pos->pList[pce][0];
	scoreMG -= KingMG[MIRROR64(SQ64(sq))];
	scoreEG -= KingEG[MIRROR64(SQ64(sq))];

	if(pos->pceNum[WHITE_BISHOP] >= 2) {
		scoreMG += BishopPairMG;
		scoreEG += BishopPairEG;
	}
	if(pos->pceNum[BLACK_BISHOP] >= 2) {
		scoreMG -= BishopPairMG;
		scoreEG -= BishopPairEG;
	}

	// scale king safety by material non-linearly
	kingScoreW = (kingScoreW * TropismMatAdjs[MIN(wPhase, 12)]) / 256;
	kingScoreB = (kingScoreB * TropismMatAdjs[MIN(bPhase, 12)]) / 256;

	scoreMG += kingScoreW + kingScoreB;
	scoreEG += kingScoreW + kingScoreB;

	// calculating game phase and interpolating score values between phases
	phase = (phase * 256 + (totalPhase / 2)) / totalPhase;
	int score = ((scoreMG * (256 - phase)) + (scoreEG * phase)) / 256;

	return pos->side == WHITE ? score : -score;

}