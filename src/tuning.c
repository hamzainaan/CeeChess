#include "stdio.h"
#include "defs.h"
#include "config.h"
#include "tuning.h"
#include "string.h"
#include "math.h"
#include "time.h"

static int MaterialDrawTunable(const S_BOARD *pos) {

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

static int EvalPositionTunable(S_BOARD *pos, S_EVAL_PARAMS *params) {
	// test for drawn position before doing anything
	if(!pos->pceNum[WHITE_PAWN] && !pos->pceNum[BLACK_PAWN] && MaterialDrawTunable(pos) == 1) {
		return 0;
	}

	int diagonal_bonus;
	int pce;
	int pceNum;
	int sq;
	int phase = TtotalPhase;
	int wPhase = 0;
	int bPhase = 0;
	int scoreMG = 0;
	int scoreEG = 0;

	int kingScoreW = 0;
	int kingScoreB = 0;

	pce = WHITE_KING;
	sq = pos->pList[pce][0];
	int wKsq64 = SQ64(sq);

	scoreMG += params->TKingMG[SQ64(sq)];
	scoreEG += params->TKingEG[SQ64(sq)];

	// if there are semi-open files near this king, boost attack score for enemy
	for (int i_sq = sq - 8; i_sq <= sq + 8; i_sq += 8) {
		if (FilesBrd[i_sq] == OFFBOARD)
			continue;
		kingScoreB -= params->TKingSemiOpen * !(pos->pawns[WHITE] & FileBBMask[FilesBrd[i_sq]]);
	}

	pce = BLACK_KING;
	sq = pos->pList[pce][0];
	int bKsq64 = SQ64(sq);

	scoreMG -= params->TKingMG[MIRROR64(SQ64(sq))];
	scoreEG -= params->TKingEG[MIRROR64(SQ64(sq))];

	// if there are semi-open files near this king, boost attack score for enemy
	for (int i_sq = sq - 8; i_sq <= sq + 8; i_sq += 8) {
		if (FilesBrd[i_sq] == OFFBOARD)
			continue;
		kingScoreW += params->TKingSemiOpen * !(pos->pawns[BLACK] & FileBBMask[FilesBrd[i_sq]]);
	}

	pce = WHITE_PAWN;
	scoreMG += pos->pceNum[pce] * params->TPieceValMG[0];
	scoreEG += pos->pceNum[pce] * params->TPieceValEG[0];
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];

		int passed = 0;
		int connected = 0;

		scoreMG += params->TPawnMG[SQ64(sq)];
		scoreEG += params->TPawnEG[SQ64(sq)];

		if( (IsolatedMask[SQ64(sq)] & pos->pawns[WHITE]) == 0) {
			//printf("WHITE_PAWN Iso:%s\n",PrSq(sq));
			scoreMG += params->TPawnIsolatedMG;
			scoreEG += params->TPawnIsolatedEG;
		}

		if( (WhitePassedMask[SQ64(sq)] & pos->pawns[BLACK]) == 0) {
			//printf("WHITE_PAWN Passed:%s\n",PrSq(sq));
			passed = 1;
			scoreMG += params->TPawnPassedMG[RanksBrd[sq]];
			scoreEG += params->TPawnPassedEG[RanksBrd[sq]];
		}

		if ((WhiteConnectedMask[SQ64(sq)] & pos->pawns[WHITE]) != 0) {
			connected = 1;
			scoreMG += params->TPawnConnectedMG;
			scoreEG += params->TPawnConnectedEG;
		}

		if (passed && connected) {
			scoreMG += params->TPawnPassedConnectedMG[RanksBrd[sq]];
			scoreEG += params->TPawnPassedConnectedEG[RanksBrd[sq]];
		}

	}

	pce = BLACK_PAWN;
	scoreMG -= pos->pceNum[pce] * params->TPieceValMG[0];
	scoreEG -= pos->pceNum[pce] * params->TPieceValEG[0];
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];

		int passed = 0;
		int connected = 0;

		scoreMG -= params->TPawnMG[MIRROR64(SQ64(sq))];
		scoreEG -= params->TPawnEG[MIRROR64(SQ64(sq))];

		if( (IsolatedMask[SQ64(sq)] & pos->pawns[BLACK]) == 0) {
			//printf("BLACK_PAWN Iso:%s\n",PrSq(sq));
			scoreMG -= params->TPawnIsolatedMG;
			scoreEG -= params->TPawnIsolatedEG;
		}

		if( (BlackPassedMask[SQ64(sq)] & pos->pawns[WHITE]) == 0) {
			//printf("BLACK_PAWN Passed:%s\n",PrSq(sq));
			passed = 1;
			scoreMG -= params->TPawnPassedMG[7 - RanksBrd[sq]];
			scoreEG -= params->TPawnPassedEG[7 - RanksBrd[sq]];
		}

		if ((BlackConnectedMask[SQ64(sq)] & pos->pawns[BLACK]) != 0) {
			connected = 1;
			scoreMG -= params->TPawnConnectedMG;
			scoreEG -= params->TPawnConnectedEG;
		}

		if (passed && connected) {
			scoreMG -= params->TPawnPassedConnectedMG[7 - RanksBrd[sq]];
			scoreEG -= params->TPawnPassedConnectedEG[7 - RanksBrd[sq]];
		}
	}

	pce = WHITE_KNIGHT;
	scoreMG += pos->pceNum[pce] * params->TPieceValMG[1];
	scoreEG += pos->pceNum[pce] * params->TPieceValEG[1];
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];
		scoreMG += params->TKnightMG[SQ64(sq)];
		scoreEG += params->TKnightEG[SQ64(sq)];
		phase -= TminorPhase;
		wPhase += TminorPhase;
		kingScoreW += (params->TTropismValues[0] * DistTable[SQ64(sq)][bKsq64]) / 16;
	}

	pce = BLACK_KNIGHT;
	scoreMG -= pos->pceNum[pce] * params->TPieceValMG[1];
	scoreEG -= pos->pceNum[pce] * params->TPieceValEG[1];
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];
		scoreMG -= params->TKnightMG[MIRROR64(SQ64(sq))];
		scoreEG -= params->TKnightEG[MIRROR64(SQ64(sq))];
		phase -= TminorPhase;
		bPhase += TminorPhase;
		kingScoreB -= (params->TTropismValues[0] * DistTable[SQ64(sq)][wKsq64]) / 16;
	}

	pce = WHITE_BISHOP;
	scoreMG += pos->pceNum[pce] * params->TPieceValMG[2];
	scoreEG += pos->pceNum[pce] * params->TPieceValEG[2];
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];
		scoreMG += params->TBishopMG[SQ64(sq)];
		scoreEG += params->TBishopEG[SQ64(sq)];
		phase -= TminorPhase;
		wPhase += TminorPhase;
		diagonal_bonus = Tbonus_dia_distance[abs(Tdiag_ne[SQ64(sq)] - Tdiag_ne[bKsq64])] + Tbonus_dia_distance[abs(Tdiag_nw[SQ64(sq)] - Tdiag_nw[bKsq64])];
		kingScoreW += (params->TTropismValues[1] * (DistTable[SQ64(sq)][bKsq64] + diagonal_bonus)) / 16;
	}

	pce = BLACK_BISHOP;
	scoreMG -= pos->pceNum[pce] * params->TPieceValMG[2];
	scoreEG -= pos->pceNum[pce] * params->TPieceValEG[2];
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];
		scoreMG -= params->TBishopMG[MIRROR64(SQ64(sq))];
		scoreEG -= params->TBishopEG[MIRROR64(SQ64(sq))];
		phase -= TminorPhase;
		bPhase += TminorPhase;
		diagonal_bonus = Tbonus_dia_distance[abs(Tdiag_ne[SQ64(sq)] - Tdiag_ne[wKsq64])] + Tbonus_dia_distance[abs(Tdiag_nw[SQ64(sq)] - Tdiag_nw[wKsq64])];
		kingScoreB -= (params->TTropismValues[1] * (DistTable[SQ64(sq)][wKsq64] + diagonal_bonus)) / 16;
	}

	pce = WHITE_ROOK;
	scoreMG += pos->pceNum[pce] * params->TPieceValMG[3];
	scoreEG += pos->pceNum[pce] * params->TPieceValEG[3];
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];
		scoreMG += params->TRookMG[SQ64(sq)];
		scoreEG += params->TRookEG[SQ64(sq)];

		if(!(pos->pawns[BOTH] & FileBBMask[FilesBrd[sq]])) {
			scoreMG += params->TRookOpenFileMG;
			scoreEG += params->TRookOpenFileEG;
		} else if(!(pos->pawns[WHITE] & FileBBMask[FilesBrd[sq]])) {
			scoreMG += params->TRookSemiOpenFileMG;
			scoreEG += params->TRookSemiOpenFileEG;
		}
		phase -= TrookPhase;
		wPhase += TrookPhase;
		kingScoreW += (params->TTropismValues[2] * DistTable[SQ64(sq)][bKsq64]) / 16;
	}

	pce = BLACK_ROOK;
	scoreMG -= pos->pceNum[pce] * params->TPieceValMG[3];
	scoreEG -= pos->pceNum[pce] * params->TPieceValEG[3];
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];
		scoreMG -= params->TRookMG[MIRROR64(SQ64(sq))];
		scoreEG -= params->TRookEG[MIRROR64(SQ64(sq))];

		if(!(pos->pawns[BOTH] & FileBBMask[FilesBrd[sq]])) {
			scoreMG -= params->TRookOpenFileMG;
			scoreEG -= params->TRookOpenFileEG;
		} else if(!(pos->pawns[BLACK] & FileBBMask[FilesBrd[sq]])) {
			scoreMG -= params->TRookSemiOpenFileEG;
			scoreEG -= params->TRookSemiOpenFileEG;
		}
		phase -= TrookPhase;
		bPhase += TrookPhase;
		kingScoreB -= (params->TTropismValues[2] * DistTable[SQ64(sq)][wKsq64]) / 16;
	}

	pce = WHITE_QUEEN;
	scoreMG += pos->pceNum[pce] * params->TPieceValMG[4];
	scoreEG += pos->pceNum[pce] * params->TPieceValEG[4];
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];
		scoreMG += params->TQueenMG[SQ64(sq)];
		scoreEG += params->TQueenEG[SQ64(sq)];

		if(!(pos->pawns[BOTH] & FileBBMask[FilesBrd[sq]])) {
			scoreMG += params->TQueenOpenFileMG;
			scoreEG += params->TQueenOpenFileEG;
		} else if(!(pos->pawns[WHITE] & FileBBMask[FilesBrd[sq]])) {
			scoreMG += params->TQueenSemiOpenFileMG;
			scoreEG += params->TQueenSemiOpenFileEG;
		}
		phase -= TqueenPhase;
		wPhase += TqueenPhase;
		diagonal_bonus = Tbonus_dia_distance[abs(Tdiag_ne[SQ64(sq)] - Tdiag_ne[bKsq64])] + Tbonus_dia_distance[abs(Tdiag_nw[SQ64(sq)] - Tdiag_nw[bKsq64])];
		kingScoreW += (params->TTropismValues[3] * (DistTable[SQ64(sq)][bKsq64] + diagonal_bonus)) / 16;
	}

	pce = BLACK_QUEEN;
	scoreMG -= pos->pceNum[pce] * params->TPieceValMG[4];
	scoreEG -= pos->pceNum[pce] * params->TPieceValEG[4];
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pList[pce][pceNum];
		scoreMG -= params->TQueenMG[MIRROR64(SQ64(sq))];
		scoreEG -= params->TQueenEG[MIRROR64(SQ64(sq))];

		if(!(pos->pawns[BOTH] & FileBBMask[FilesBrd[sq]])) {
			scoreMG -= params->TQueenOpenFileMG;
			scoreEG -= params->TQueenOpenFileEG;
		} else if(!(pos->pawns[BLACK] & FileBBMask[FilesBrd[sq]])) {
			scoreMG -= params->TQueenSemiOpenFileMG;
			scoreEG -= params->TQueenSemiOpenFileEG;
		}
		phase -= TqueenPhase;
		bPhase += TqueenPhase;
		diagonal_bonus = Tbonus_dia_distance[abs(Tdiag_ne[SQ64(sq)] - Tdiag_ne[wKsq64])] + Tbonus_dia_distance[abs(Tdiag_nw[SQ64(sq)] - Tdiag_nw[wKsq64])];
		kingScoreB -= (params->TTropismValues[3] * (DistTable[SQ64(sq)][wKsq64] + diagonal_bonus)) / 16;
	}

	if(pos->pceNum[WHITE_BISHOP] >= 2) {
		scoreMG += params->TBishopPairMG;
		scoreEG += params->TBishopPairEG;
	}
	if(pos->pceNum[BLACK_BISHOP] >= 2) {
		scoreMG -= params->TBishopPairMG;
		scoreEG -= params->TBishopPairEG;
	}

	// scale king safety by material non-linearly
	kingScoreW = (kingScoreW * params->TTropismMatAdjs[MIN(wPhase, 12)]) / 256;
	kingScoreB = (kingScoreB * params->TTropismMatAdjs[MIN(bPhase, 12)]) / 256;

	scoreMG += kingScoreW + kingScoreB;
	scoreEG += kingScoreW + kingScoreB;

	// calculating game phase and interpolating score values between phases
	phase = (phase * 256 + (TtotalPhase / 2)) / TtotalPhase;
	int score = ((scoreMG * (256 - phase)) + (scoreEG * phase)) / 256;

	// For tuning, instead of returning the score based on the side to move, return the score in terms of white
	return score;
}

void printBar(int size, char* bar) {
	for (int i = 0; i < size; i++) {
		printf("%s", bar);
	}
	printf("\n");
}

void printProgressBar(int progress, int total, char* title) {
	int width = 55;
    printf("%s [", title);
    int pos = width * progress / total;  // Calculate the position for the progress bar
    for (int i = 0; i < width; ++i) {
        if (i < pos) {
            printf("=");
        } else if (i == pos) {
            printf(">");
        } else {
            printf(" ");
        }
    }
    printf("] %d/%d\r", progress, total);  // '\r' moves the cursor to the beginning of the line
    fflush(stdout);  // Flush the output to make sure the progress bar is immediately displayed
}

static int count_lines(FILE* file)
{
    char buf[256];
    int counter = 1;
    for(;;)
    {
        size_t res = fread(buf, 1, 256, file);
        if (ferror(file))
            return -1;

        int i;
        for(i = 0; i < res; i++)
            if (buf[i] == '\n')
                counter++;

        if (feof(file))
            break;
    }
	fseek(file, 0, SEEK_SET);
    return counter;
}

static void paramsToList(S_EVAL_PARAMS *params, int* paramsList) {
	memcpy(paramsList, params, sizeof(S_EVAL_PARAMS));
}

static void listToParams(int* paramsList, S_EVAL_PARAMS *params) {
	memcpy(params, paramsList, sizeof(S_EVAL_PARAMS));
}

static void initParams(S_EVAL_PARAMS *params) {
	int matVals[5] = { 100, 325, 325, 550, 1000 };
	memcpy(params->TPieceValMG, matVals, sizeof(matVals));
    params->TBishopPairMG = TBishopPairMG;
	memcpy(params->TPawnPassedMG, TPawnPassedMG, sizeof(TPawnPassedMG));
	memcpy(params->TPawnPassedConnectedMG, TPawnPassedConnectedMG, sizeof(TPawnPassedConnectedMG));
    params->TPawnConnectedMG = TPawnConnectedMG;
    params->TPawnIsolatedMG = TPawnIsolatedMG;
    params->TRookOpenFileMG = TRookOpenFileMG;
    params->TRookSemiOpenFileMG = TRookSemiOpenFileMG;
    params->TQueenOpenFileMG = TQueenOpenFileMG;
    params->TQueenSemiOpenFileMG = TQueenSemiOpenFileMG;

	memcpy(params->TPieceValEG, matVals, sizeof(matVals));
    params->TBishopPairEG = TBishopPairEG;
	memcpy(params->TPawnPassedEG, TPawnPassedEG, sizeof(TPawnPassedEG));
	memcpy(params->TPawnPassedConnectedEG, TPawnPassedConnectedEG, sizeof(TPawnPassedConnectedEG));
    params->TPawnConnectedEG = TPawnConnectedEG;
    params->TPawnIsolatedEG = TPawnIsolatedEG;
    params->TRookOpenFileEG = TRookOpenFileEG;
    params->TRookSemiOpenFileEG = TRookSemiOpenFileEG;
    params->TQueenOpenFileEG = TQueenOpenFileEG;
    params->TQueenSemiOpenFileEG = TQueenSemiOpenFileEG;

	params->TKingSemiOpen = TKingSemiOpen;
	memcpy(params->TTropismValues, TTropismValues, sizeof(TTropismValues));
	memcpy(params->TTropismMatAdjs, TTropismMatAdjs, sizeof(TTropismMatAdjs));

	memcpy(params->TPawnMG, TPawnMG, sizeof(TPawnMG));
	memcpy(params->TPawnEG, TPawnEG, sizeof(TPawnEG));
	memcpy(params->TKnightMG, TKnightMG, sizeof(TKnightMG));
	memcpy(params->TKnightEG, TKnightEG, sizeof(TKnightEG));
	memcpy(params->TBishopMG, TBishopMG, sizeof(TBishopMG));
	memcpy(params->TBishopEG, TBishopEG, sizeof(TBishopEG));
	memcpy(params->TRookMG, TRookMG, sizeof(TRookMG));
	memcpy(params->TRookEG, TRookEG, sizeof(TRookEG));
	memcpy(params->TQueenMG, TQueenMG, sizeof(TQueenMG));
	memcpy(params->TQueenEG, TQueenEG, sizeof(TQueenEG));
	memcpy(params->TKingMG, TKingMG, sizeof(TKingMG));
	memcpy(params->TKingEG, TKingEG, sizeof(TKingEG));
}

void readFENScore(char fen[MAX_FEN_LEN], double *score, FILE* inputFile) {
	int character;
	int index = 0;
    while ((character = fgetc(inputFile)) != EOF && character != '[') {
        fen[index++] = (char)character;
    }
	fscanf(inputFile, "%lf]\n", score);
}

void readFENScores(char (*fenBuf)[MAX_FEN_LEN], double *scores, FILE* inputFile, int numPos) {
	for (int i = 0; i < numPos; i++) {
		readFENScore(fenBuf[i], &scores[i], inputFile);
	}
}

static double getError(S_BOARD *pos, S_EVAL_PARAMS *params, double K, S_BOARD *boards, double *scores, int numPos, int posOffset, int isTanh) {
	double total_error = 0.0;
	
	// Use pre-parsed boards instead of parsing FEN each time
	#pragma omp parallel reduction(+:total_error)
	{
		#pragma omp for schedule(dynamic)
		for (int i = posOffset; i < numPos + posOffset; i++) {
			double result = scores[i];
			// evaluate the position using pre-parsed board
			int score = EvalPositionTunable(&boards[i], params);
			double sigmoid = 1 / (1 + pow(10, ((-K * score) / 400)));
			// this is incorrect for the stockfish augmented dataset so commented out
			double tanhResult = 2 * result - 1;
			//double tanhResult = result;
			double tanh_eval = tanh((K * score) / 400);
			// add to error (error is based on pseudo-huber loss, to reduce the power of outliers)
			double residual = (isTanh) ? tanhResult - tanh_eval : result - sigmoid;
			double local_error = 0.25 * (sqrt(1 + (2 * pow(residual, 2))) - 1);
			total_error += local_error;
		}
	}
	return total_error / numPos;
}

// Pre-allocated batch indices for memory efficiency
static int *sa_batch_indices = NULL;
static int *ls_batch_indices = NULL;

// Initialize batch indices for memory efficiency
void initBatchIndices(int sa_size, int ls_size, int numPos, int posOffset) {
	// Free previous allocations if they exist
	if (sa_batch_indices != NULL) {
		free(sa_batch_indices);
		sa_batch_indices = NULL;
	}
	if (ls_batch_indices != NULL) {
		free(ls_batch_indices);
		ls_batch_indices = NULL;
	}
	
	// Allocate and initialize SA batch indices
	sa_batch_indices = (int *)malloc(sa_size * sizeof(int));
	if (sa_batch_indices == NULL) {
		printf("Error: Failed to allocate memory for SA batch indices\n");
		return;
	}
	
	// Allocate and initialize LS batch indices
	ls_batch_indices = (int *)malloc(ls_size * sizeof(int));
	if (ls_batch_indices == NULL) {
		printf("Error: Failed to allocate memory for LS batch indices\n");
		free(sa_batch_indices);
		sa_batch_indices = NULL;
		return;
	}
	
	// Initialize with random indices
	for (int i = 0; i < sa_size; i++) {
		sa_batch_indices[i] = posOffset + (rand() % numPos);
	}
	
	for (int i = 0; i < ls_size; i++) {
		ls_batch_indices[i] = posOffset + (rand() % numPos);
	}
}

// Free batch indices memory
void freeBatchIndices() {
	if (sa_batch_indices != NULL) {
		free(sa_batch_indices);
		sa_batch_indices = NULL;
	}
	if (ls_batch_indices != NULL) {
		free(ls_batch_indices);
		ls_batch_indices = NULL;
	}
}

// Refresh batch indices with new random values
void refreshBatchIndices(int batch_type, int batch_size, int numPos, int posOffset) {
	int *indices = (batch_type == 0) ? sa_batch_indices : ls_batch_indices;
	
	if (indices == NULL) {
		printf("Error: Batch indices not initialized\n");
		return;
	}
	
	for (int i = 0; i < batch_size; i++) {
		indices[i] = posOffset + (rand() % numPos);
	}
}

// Mini-batch version of getError that uses a random subset of positions
static double getErrorMiniBatch(S_BOARD *pos, S_EVAL_PARAMS *params, double K, S_BOARD *boards, double *scores, 
							int numPos, int posOffset, int isTanh, int batch_size) {
	double total_error = 0.0;
	
	// Determine which batch indices to use (0 for SA, 1 for LS)
	int batch_type = (batch_size > 50000) ? 0 : 1; // Assume larger batch is SA, smaller is LS
	int *indices = (batch_type == 0) ? sa_batch_indices : ls_batch_indices;
	
	// Check if indices are initialized
	if (indices == NULL) {
		// Fallback to temporary allocation if not initialized
		indices = (int *)malloc(batch_size * sizeof(int));
		if (indices == NULL) {
			printf("Error: Failed to allocate memory for batch indices\n");
			return INFINITE;
		}
		
		for (int i = 0; i < batch_size; i++) {
			indices[i] = posOffset + (rand() % numPos);
		}
	}
	
	// Use pre-parsed boards instead of parsing FEN each time
	#pragma omp parallel reduction(+:total_error)
	{
		#pragma omp for schedule(dynamic)
		for (int j = 0; j < batch_size; j++) {
			int i = indices[j];
			double result = scores[i];
			// evaluate the position using pre-parsed board
			int score = EvalPositionTunable(&boards[i], params);
			double sigmoid = 1 / (1 + pow(10, ((-K * score) / 400)));
			// this is incorrect for the stockfish augmented dataset so commented out
			double tanhResult = 2 * result - 1;
			//double tanhResult = result;
			double tanh_eval = tanh((K * score) / 400);
			// add to error (error is based on pseudo-huber loss, to reduce the power of outliers)
			double residual = (isTanh) ? tanhResult - tanh_eval : result - sigmoid;
			double local_error = 0.25 * (sqrt(1 + (2 * pow(residual, 2))) - 1);
			total_error += local_error;
		}
	}
	
	// Only free if we had to allocate temporarily
	if (sa_batch_indices == NULL && ls_batch_indices == NULL) {
		free(indices);
	}
	
	return total_error / batch_size;
}

int getRandomNumber(int step_size) {
    int randomNumber = rand() % (step_size * 2 + 1);
    return randomNumber - step_size;
}

void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void shuffleIndices(int *indices, int size) {

    // Initialize indices array
    for (int i = 0; i < size; ++i) {
        indices[i] = i;
    }

	int PSQTIndex = size - (64 * 6 * 2);

	// Fisher-Yates shuffle the general indices
    for (int i = PSQTIndex; i > 0; --i) {
        int j = rand() % (i + 1);
        swap(&indices[i], &indices[j]);
    }

    // Fisher-Yates shuffle the PSQT indices
    for (int i = size - 1; i > PSQTIndex; --i) {
        int j = rand() % (i + 1);
        swap(&indices[i], &indices[j]);
    }

	// this way, the general terms are optimized before the psqt, but there is shuffling
}

void shuffleParents(int *indices, int size) {

    // Initialize indices array
    for (int i = 0; i < size; ++i) {
        indices[i] = i;
    }

    // Fisher-Yates shuffle the PSQT indices
    for (int i = size - 1; i > 0; --i) {
        int j = rand() % (i + 1);
        swap(&indices[i], &indices[j]);
    }
	// this way, the general terms are optimized before the psqt, but there is shuffling
}

static void printPSQTToFile(const int *array, FILE* file) {
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            fprintf(file, "%d, ", array[i * 8 + j]);
        }
        fprintf(file, "\n");
    }
	fprintf(file, "};\n");
}

static void printParamsToFile(S_EVAL_PARAMS *params, FILE *file) {
	for (int i = 0; i < 28; i++)
		fprintf(file, "-");
	fprintf(file, "\nCurrent Parameters: \n"); 
	for (int i = 0; i < 19; i++)
		fprintf(file, "-");
	fprintf(file, "\nconst int PieceValMG[5] = {");
    for (int i = 0; i < 5; ++i) {
        fprintf(file, "%d, ", params->TPieceValMG[i]);
    }
    fprintf(file, "};\nconst int BishopPairMG = %d;\n", params->TBishopPairMG);

    fprintf(file, "const int PawnPassedMG[8] = {");
    for (int i = 0; i < 8; ++i) {
        fprintf(file, "%d, ", params->TPawnPassedMG[i]);
    }
    fprintf(file, "};\nconst int PawnPassedConnectedMG[8] = {");
    for (int i = 0; i < 8; ++i) {
        fprintf(file, "%d, ", params->TPawnPassedConnectedMG[i]);
    }
    fprintf(file, "};\nconst int PawnConnectedMG = %d;\n", params->TPawnConnectedMG);
    fprintf(file, "const int PawnIsolatedMG = %d;\n", params->TPawnIsolatedMG);
    fprintf(file, "const int RookOpenFileMG = %d;\n", params->TRookOpenFileMG);
    fprintf(file, "const int RookSemiOpenFileMG = %d;\n", params->TRookSemiOpenFileMG);
    fprintf(file, "const int QueenOpenFileMG = %d;\n", params->TQueenOpenFileMG);
    fprintf(file, "const int QueenSemiOpenFileMG = %d;\n", params->TQueenSemiOpenFileMG);

    fprintf(file, "const int PieceValEG[5] = {");
    for (int i = 0; i < 5; ++i) {
        fprintf(file, "%d, ", params->TPieceValEG[i]);
    }
    fprintf(file, "};\nconst int BishopPairEG = %d;\n", params->TBishopPairEG);

    fprintf(file, "const int PawnPassedEG[8] = {");
    for (int i = 0; i < 8; ++i) {
        fprintf(file, "%d, ", params->TPawnPassedEG[i]);
    }
    fprintf(file, "};\nconst int PawnPassedConnectedEG[8] = {");
    for (int i = 0; i < 8; ++i) {
        fprintf(file, "%d, ", params->TPawnPassedConnectedEG[i]);
    }
    fprintf(file, "};\nconst int PawnConnectedEG = %d;\n", params->TPawnConnectedEG);
    fprintf(file, "const int PawnIsolatedEG = %d;\n", params->TPawnIsolatedEG);
    fprintf(file, "const int RookOpenFileEG = %d;\n", params->TRookOpenFileEG);
    fprintf(file, "const int RookSemiOpenFileEG = %d;\n", params->TRookSemiOpenFileEG);
    fprintf(file, "const int QueenOpenFileEG = %d;\n", params->TQueenOpenFileEG);
    fprintf(file, "const int QueenSemiOpenFileEG = %d;\n", params->TQueenSemiOpenFileEG);

	fprintf(file, "const int KingSemiOpen = %d;\n", params->TKingSemiOpen);
	fprintf(file, "const int TropismValues[4] = {");
    for (int i = 0; i < 4; ++i) {
        fprintf(file, "%d, ", params->TTropismValues[i]);
    }
	fprintf(file, "};\nconst int TropismMatAdjs[13] = {");
    for (int i = 0; i < 13; ++i) {
        fprintf(file, "%d, ", params->TTropismMatAdjs[i]);
    }

    fprintf(file, "};\nconst int PawnMG[64] = {\n");
    printPSQTToFile(params->TPawnMG, file);
    fprintf(file, "const int PawnEG[64] = {\n");
    printPSQTToFile(params->TPawnEG, file);
    fprintf(file, "const int KnightMG[64] = {\n");
    printPSQTToFile(params->TKnightMG, file);
    fprintf(file, "const int KnightEG[64] = {\n");
    printPSQTToFile(params->TKnightEG, file);
    fprintf(file, "const int BishopMG[64] = {\n");
    printPSQTToFile(params->TBishopMG, file);
    fprintf(file, "const int BishopEG[64] = {\n");
    printPSQTToFile(params->TBishopEG, file);
    fprintf(file, "const int RookMG[64] = {\n");
    printPSQTToFile(params->TRookMG, file);
    fprintf(file, "const int RookEG[64] = {\n");
    printPSQTToFile(params->TRookEG, file);
    fprintf(file, "const int QueenMG[64] = {\n");
    printPSQTToFile(params->TQueenMG, file);
    fprintf(file, "const int QueenEG[64] = {\n");
    printPSQTToFile(params->TQueenEG, file);
    fprintf(file, "const int KingMG[64] = {\n");
    printPSQTToFile(params->TKingMG, file);
    fprintf(file, "const int KingEG[64] = {\n");
    printPSQTToFile(params->TKingEG, file);
    fprintf(file, "\n");
}

#ifdef _OPENMP
#include <omp.h>
#endif

// Parse FENs in chunks to avoid memory issues
#define PARSE_CHUNK_SIZE 500000 // Process 500K positions at a time

void parseFENsToBoards(char (*fenBuf)[MAX_FEN_LEN], S_BOARD *boards, int numPos) {
	int chunks = (numPos + PARSE_CHUNK_SIZE - 1) / PARSE_CHUNK_SIZE; // Ceiling division
	printf("Parsing in %d chunks to manage memory usage...\n", chunks);
	
	// Track progress and performance
	time_t start_time = time(NULL);
	int total_parsed = 0;
	
	for (int chunk = 0; chunk < chunks; chunk++) {
		int start = chunk * PARSE_CHUNK_SIZE;
		int end = MIN((chunk + 1) * PARSE_CHUNK_SIZE, numPos);
		int chunkSize = end - start;
		
		printf("Parsing chunk %d/%d (%d positions, %d-%d)...\n", 
			chunk + 1, chunks, chunkSize, start, end - 1);
		
		#pragma omp parallel
		{
			#pragma omp for schedule(dynamic)
			for (int i = start; i < end; i++) {
				ParseFen(fenBuf[i], &boards[i]);
				
				// Track progress (thread-safe)
				#pragma omp critical
				{
					total_parsed++;
					if (total_parsed % 50000 == 0) {
							time_t current = time(NULL);
							double elapsed = difftime(current, start_time);
							printf("Processed %d/%d positions (%.1f%%) - %.1f pos/sec\r", 
								total_parsed, numPos, 
								(double)total_parsed*100/numPos,
								(elapsed > 0) ? (total_parsed / elapsed) : 0);
							fflush(stdout);
					}
				}
			}
		}
		
		// Force memory cleanup after each chunk
		#pragma omp barrier
		printf("\nChunk %d/%d completed.\n", chunk + 1, chunks);
	}
	
	time_t end_time = time(NULL);
	printf("Parsing complete in %.1f seconds.\n", difftime(end_time, start_time));
}

void TuneEval(S_BOARD *pos, char *fileIn, char *fileOut, char *fileLog, int use_tanh) {
	int adjust_val = 1;
	double K = 1;
	int useTanh = use_tanh;

	// Set up OpenMP for multi-core processing
	#ifdef _OPENMP
	// Use all available cores
	int num_cores = omp_get_num_procs();
	omp_set_num_threads(num_cores);
	printf("Using %d cores for tuning process\n", num_cores);
	#endif

	FILE *input = fopen(fileIn, "r");
	if (input == NULL) {
		printf("Error: Could not open input file %s\n", fileIn);
		return;
	}
	int numPos = count_lines(input);

	printf("Opened %s, with %d positions\n", fileIn, numPos);

	S_EVAL_PARAMS params[1];
	initParams(params);

	FILE* out = fopen(fileOut, "w+");
	if (out == NULL) {
		printf("Error: Could not open output file %s\n", fileOut);
		fclose(input);
		return;
	}
	fclose(out);

	FILE* log = fopen(fileLog, "w+");
	if (log == NULL) {
		printf("Error: Could not open log file %s\n", fileLog);
		fclose(input);
		return;
	}
	fclose(log);

	
	int *params_list = (int *)malloc(sizeof(S_EVAL_PARAMS));
	int *new_params_list = (int *)malloc(sizeof(S_EVAL_PARAMS));
	int params_length = sizeof(S_EVAL_PARAMS) / sizeof(params_list[0]);

	// start with current weights
	paramsToList(params, params_list);

	// read FENs + scores in
	char (*fens_buf)[MAX_FEN_LEN] = malloc(sizeof(char[numPos][MAX_FEN_LEN]));
	double *scores_buf = (double *)malloc(numPos * sizeof(double));
	readFENScores(fens_buf, scores_buf, input, numPos);
	fclose(input);

	// Allocate memory for boards - check if allocation is successful
	printf("Allocating memory for %d positions...\n", numPos);
	S_BOARD *boards = (S_BOARD *)malloc(numPos * sizeof(S_BOARD));
	if (boards == NULL) {
		printf("Error: Failed to allocate memory for boards. Try using a smaller dataset.\n");
		free(fens_buf);
		free(scores_buf);
		return;
	}

	// Parse FENs in chunks to manage memory usage
	parseFENsToBoards(fens_buf, boards, numPos);
	printf("Parsing complete.\n");

	// get index of training set and validation set (90 - 10 split)
	int numPosTrain = 0.9 * numPos;
	int numPosVal = numPos - numPosTrain;
	int valOffset = numPosTrain;

	// Calculate mini-batch sizes
	int sa_batch_size = MIN(100000, numPosTrain / 100);
	int ls_batch_size = 50000;
	printf("Using mini-batch sizes: SA=%d, LS=%d\n", sa_batch_size, ls_batch_size);
	
	// Initialize batch indices for memory efficiency
	printf("Initializing batch indices...\n");
	initBatchIndices(sa_batch_size, ls_batch_size, numPosTrain, 0);

	double last_error = INFINITE;
	// Use full dataset for initial error calculation
	double best_error = getError(pos, params, K, boards, scores_buf, numPosTrain, 0, useTanh);

	// first tune the K via local search with mini-batch
	printf("Optimizing K before tuning: \n");
	double K_adj_val = 0.1;
	while (best_error < last_error) {
		last_error = best_error;
		printf("Local Search - K: %0.03lf\r", K);
		K += K_adj_val;
		listToParams(params_list, params);
		// Use mini-batch for K optimization
		double new_error = getErrorMiniBatch(pos, params, K, boards, scores_buf, numPosTrain, 0, useTanh, ls_batch_size);
		if (new_error < best_error) {
			// Verify with full dataset if mini-batch shows improvement
			new_error = getError(pos, params, K, boards, scores_buf, numPosTrain, 0, useTanh);
			if (new_error < best_error) {
				best_error = new_error;
				continue;
			}
		}
		K -= 2 * K_adj_val;
		listToParams(params_list, params);
		// Use mini-batch for K optimization
		new_error = getErrorMiniBatch(pos, params, K, boards, scores_buf, numPosTrain, 0, useTanh, ls_batch_size);
		if (new_error < best_error) {
			// Verify with full dataset if mini-batch shows improvement
			new_error = getError(pos, params, K, boards, scores_buf, numPosTrain, 0, useTanh);
			if (new_error < best_error) {
				best_error = new_error;
				continue;
			}
		}
		K += K_adj_val; // reset if failed
	}
	printf("\n");

	// show initial error
	printf("Initial error before tuning: %0.06lf\n", best_error);

	// then tune with simulated annealing (for exploration) and local search (for exploitation)
	int epoch = 0;
	double init_temp = 1e-3; // başlangıç, hata ölçeğine yakın (hata ~0.022)
	double cooling_rate = 0.9995; // çok yavaş soğuma
	double temperature = init_temp; // her epoch başında reheat veya kullan
	int reheat_interval = 5; // her 5 epoch'ta bir reheat
	do {
		epoch++;
		last_error = best_error;
		printf("Epoch %d: \n", epoch);

		// Her reheat_interval epoch'ta temperature'ı yeniden başlat
		if (epoch % reheat_interval == 0) {
			temperature = init_temp; // reheat
			printf(" - Reheating temperature to %0.06lf\n", temperature);
		}

		// Her epoch'ta simulated annealing çalıştır
		// explorative part of the algorithm, so annealing parameters are tuned for high exploration
			int sim_iters = 2000; // mini-batch kullandığımız için azaltıldı
			double tune_probability = 0.05;
			int tune_size = 2;
			printf(" - Simulated Annealing (mini-batch size: %d)\n", sa_batch_size);
			
			// Refresh SA batch indices for this epoch
			refreshBatchIndices(0, sa_batch_size, numPosTrain, 0);
			for (int i = 0; i < sim_iters; i++) {
				for (int i = 0; i < params_length; i++) {
					double random_number = rand() / (double)RAND_MAX;
					if (random_number < tune_probability)
						new_params_list[i] = params_list[i] + getRandomNumber(tune_size);
					else
						new_params_list[i] = params_list[i];
				}
				listToParams(new_params_list, params);
				// Use mini-batch for SA
				double new_error = getErrorMiniBatch(pos, params, K, boards, scores_buf, numPosTrain, 0, useTanh, sa_batch_size);
				double random_number = rand() / (double)RAND_MAX;
				if (new_error < best_error || random_number < exp((best_error - new_error) / temperature)) {
					// If mini-batch shows improvement, verify with full dataset
					if (new_error < best_error) {
						double full_error = getError(pos, params, K, boards, scores_buf, numPosTrain, 0, useTanh);
						if (full_error < best_error) {
							best_error = full_error;
							memcpy(params_list, new_params_list, sizeof(S_EVAL_PARAMS));
						}
					} else {
						// For exploration (random acceptance), use mini-batch error
						best_error = new_error;
						memcpy(params_list, new_params_list, sizeof(S_EVAL_PARAMS));
					}
				}
				temperature *= cooling_rate; // Çok yavaş soğuma (0.9995)
				printf(" --- Iteration %3d - Temp %0.06lf - Error: %0.06lf\r", i + 1, temperature, best_error);
			}
		
		printf("\n");
		//printParams(params);

		// then perform local search, with a max step size max_in_a_row 
		// exploitative part of the algorithm
		int steps_in_a_row = 0;
		int max_in_a_row = MIN((epoch / 2), 10); // slowly increase exploitaion as algorithm goes on
		printf(" - Shuffled Local Search (mini-batch size: %d)\n", ls_batch_size);
		
		// Refresh LS batch indices for this epoch
		refreshBatchIndices(1, ls_batch_size, numPosTrain, 0);
		
		int *shuffled_indices = (int *)malloc(params_length * sizeof(int));
		shuffleIndices(shuffled_indices, params_length);
		for (int i = 0; i < params_length; i++) {
			printf(" --- Parameter %3d/%d ---------- Error: %0.06lf\r", i + 1, params_length, best_error);
			params_list[shuffled_indices[i]] += adjust_val;
			listToParams(params_list, params);
			// Use mini-batch for local search
			double new_error = getErrorMiniBatch(pos, params, K, boards, scores_buf, numPosTrain, 0, useTanh, ls_batch_size);
			if (new_error < best_error) {
				// If mini-batch shows improvement, verify with full dataset when we have consecutive improvements
				steps_in_a_row++;
				if (steps_in_a_row >= max_in_a_row) {
					// Verify with full dataset after consecutive improvements
					double full_error = getError(pos, params, K, boards, scores_buf, numPosTrain, 0, useTanh);
					if (full_error < best_error) {
						best_error = full_error;
						i--;
						steps_in_a_row = 0; // Reset counter after full evaluation
					} else {
						// If full dataset doesn't confirm improvement, reset parameter
						params_list[shuffled_indices[i]] -= adjust_val;
						listToParams(params_list, params);
						steps_in_a_row = 0;
					}
				} else {
					// Use mini-batch error for consecutive improvements tracking
					best_error = new_error;
					i--;
				}
				continue;
			}
			params_list[shuffled_indices[i]] -= 2 * adjust_val;
			listToParams(params_list, params);
			// Use mini-batch for local search
			new_error = getErrorMiniBatch(pos, params, K, boards, scores_buf, numPosTrain, 0, useTanh, ls_batch_size);
			if (new_error < best_error) {
				// If mini-batch shows improvement, verify with full dataset when we have consecutive improvements
				steps_in_a_row++;
				if (steps_in_a_row >= max_in_a_row) {
					// Verify with full dataset after consecutive improvements
					double full_error = getError(pos, params, K, boards, scores_buf, numPosTrain, 0, useTanh);
					if (full_error < best_error) {
						best_error = full_error;
						i--;
						steps_in_a_row = 0; // Reset counter after full evaluation
					} else {
						// If full dataset doesn't confirm improvement, reset parameter
						params_list[shuffled_indices[i]] += 2 * adjust_val;
						listToParams(params_list, params);
						steps_in_a_row = 0;
					}
				} else {
					// Use mini-batch error for consecutive improvements tracking
					best_error = new_error;
					i--;
				}
				continue;
			}
			params_list[shuffled_indices[i]] += adjust_val; // reset if failed
			steps_in_a_row = 0; // Reset counter when no improvement
		}
		printf("\n");
		// add to log file
		FILE* log = fopen(fileLog, "a");
		for (int i = 0; i < 28; i++)
			fprintf(log, "-");
		// test final errors with full dataset
		double train_error = getError(pos, params, K, boards, scores_buf, numPosTrain, 0, useTanh);
		double valid_error = getError(pos, params, K, boards, scores_buf, numPosVal, valOffset, useTanh);
		printf(" - Train Error: %lf\n - Valid Error: %lf\n", train_error, valid_error);
		fprintf(log, "\n--- Epoch %d ---\n - Train Error: %lf\n - Valid Error: %lf\n", epoch, train_error, valid_error);
		printParamsToFile(params, log);
		fclose(log);

		// replace best in out file
		FILE* out = fopen(fileOut, "w");
		for (int i = 0; i < 28; i++)
			fprintf(out, "-");
		fprintf(out, "\n--- Epoch %d ---\n - Train Error: %lf\n - Valid Error: %lf\n", epoch, train_error, valid_error);
		printParamsToFile(params, out);
		fclose(out);

		free(shuffled_indices);
	} while (best_error < last_error);

	listToParams(params_list, params);
	double train_error = getError(pos, params, K, boards, scores_buf, numPosTrain, 0, useTanh);
	double valid_error = getError(pos, params, K, boards, scores_buf, numPosVal, valOffset, useTanh);
	printf("Error after tuning --- Train: %0.06lf | Valid: %0.06lf", train_error, valid_error);

	printf("Tuned Parameters in %s\n", fileOut);
	
	// Free all allocated memory
	freeBatchIndices(); // Free batch indices
	free(params_list);
	free(new_params_list);
	free(fens_buf);
	free(scores_buf);
	free(boards);
	
	printf("\nMemory cleanup completed successfully.\n");
}