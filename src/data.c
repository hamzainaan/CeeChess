#include "defs.h"

char PceChar[] 				= ".PNBRQKpnbrqk";
char SideChar[] 			= "wb-";
char RankChar[] 			= "12345678";
char FileChar[] 			= "abcdefgh";

int PieceBig[13] 			= { 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0 };
int PieceMaj[13] 			= { 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1 };
int PieceMin[13] 			= { 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0 };
int PieceValMG[13]			= { 0, 91, 377, 384, 515, 1124, 0, 91, 377, 384, 515, 1124, 0  };
int PieceValEG[13]			= { 0, 98, 327, 354, 614, 1127, 0, 98, 327, 354, 614, 1127, 0  };
int PieceCol[13] 			= { BOTH, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK };
const int KnDir[] 			= { KN_SSW, KN_SWW, KN_NNW, KN_NWW, KN_SSE, KN_SEE, KN_NNE, KN_NEE };
const int RkDir[] 			= { RK_W, RK_S, RK_E, RK_N };
const int BiDir[] 			= { BI_SW, BI_SE, BI_NW, BI_NE };
const int KiDir[] 			= { RK_W, RK_S, RK_E, RK_N, BI_SW, BI_SE, BI_NW, BI_NE };

int PiecePawn[13] 			= { 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0 };
int PieceKnight[13] 		= { 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 };
int PieceKing[13] 			= { 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1 };
int PieceRookQueen[13] 		= { 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0 };
int PieceBishopQueen[13] 	= { 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0 };
int PieceSlides[13] 		= { 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0 };

int Mirror64[64] = {
	56, 57, 58, 59, 60, 61, 62, 63,
	48, 49, 50, 51, 52, 53, 54, 55,
	40, 41, 42, 43, 44, 45, 46, 47,
	32, 33, 34, 35, 36, 37, 38, 39,
	24, 25, 26, 27, 28, 29, 30, 31,
	16, 17, 18, 19, 20, 21, 22, 23,
	 8,  9, 10, 11, 12, 13, 14, 15,
	 0,  1,  2,  3,  4,  5,  6,  7
};
