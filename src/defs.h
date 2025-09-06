#ifndef DEFS_H
#define DEFS_H

#include "stdlib.h"
#include "stdio.h"
#include "config.h"

#include <pthread.h>

typedef unsigned long long U64;

enum { EMPTY, WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, WHITE_KING, BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, BLACK_KING };
enum { FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H };
enum { RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8 };
enum { WKCA = 1, WQCA = 2, BKCA = 4, BQCA = 8 };
enum { WHITE, BLACK, BOTH };

enum {
  A1 = 21, B1, C1, D1, E1, F1, G1, H1,
  A2 = 31, B2, C2, D2, E2, F2, G2, H2,
  A3 = 41, B3, C3, D3, E3, F3, G3, H3,
  A4 = 51, B4, C4, D4, E4, F4, G4, H4,
  A5 = 61, B5, C5, D5, E5, F5, G5, H5,
  A6 = 71, B6, C6, D6, E6, F6, G6, H6,
  A7 = 81, B7, C7, D7, E7, F7, G7, H7,
  A8 = 91, B8, C8, D8, E8, F8, G8, H8, 
  NO_SQ, OFFBOARD
};

enum KnightDirection {
    KN_NNW = -21,
    KN_NWW = -19,
    KN_SWW = -12,
    KN_SSW = -8,
    KN_SSE = 8,
    KN_SEE = 12,
    KN_NEE = 19,
    KN_NNE = 21
};

enum RookDirection {
    RK_W = -1,
    RK_S = -10,
    RK_E = 1,
    RK_N = 10
};

enum BishopDirection {
    BI_SW = -11,
    BI_SE = -9,
    BI_NW = 9,
    BI_NE = 11
};

typedef struct {
	int move;
	int score;
} S_MOVE;

typedef struct {
	S_MOVE moves[MAXPOSITIONMOVES];
	int count;
} S_MOVELIST;

#ifdef _MSC_VER
__declspec(align(64))
#endif
typedef struct {
	U64 posKey;
	int move;
	int score;
	int depth;
	int flags;
	int age;
	int padding[10];
}
#ifdef __GNUC__
__attribute__((aligned(64)))
#endif
S_HASHENTRY;

typedef struct {
	S_HASHENTRY *pTable;
	size_t numEntries;
	int newWrite;
	int overWrite;
	int hit;
	int cut;
	int currentage;
} S_HASHTABLE;

typedef struct {
	int move;
	int castlePerm;
	int enPas;
	int fiftyMove;
	U64 posKey;
} S_UNDO;

typedef struct {
	int pieces[120];
	U64 pawns[3];
	int KingSq[2];
	int side;
	int enPas;
	int fiftyMove;
	int ply;
	int hisPly;
	int castlePerm;
	U64 posKey;
	int pceNum[13];
	int bigPce[2];
	int majPce[2];
	int minPce[2];
	int material[4];
	S_UNDO history[MAXGAMEMOVES];
	int pList[13][10];
	int PvArray[MAXDEPTH];
	int searchHistory[13][120];
	int searchKillers[2][MAXDEPTH];
} S_BOARD;

typedef struct {
    int starttime;
    int stoptime;
    int depth;
    int timeset;
    int movestogo;
    long nodes;
    int quit;
    int stopped;
    float fh;
    float fhf;
    int nullCut;
    int singularExt;
    int threadCount;
    int threadNum; // 0 = main thread, 1+ = worker threads
    pthread_t threadHandle;
    pthread_mutex_t mutex;
    int searching;
    int pondering; // 1 if engine is pondering, 0 otherwise
    int ponderMove;
} S_SEARCHINFO;

#define FROMSQ(m) ((m) & 0x7F)
#define TOSQ(m) (((m)>>7) & 0x7F)
#define CAPTURED(m) (((m)>>14) & 0xF)
#define PROMOTED(m) (((m)>>20) & 0xF)
#define MAX(a, b) ((a > b) ? a : b)
#define MIN(a, b) ((a < b) ? a : b)
#define FR2SQ(f,r) ( (21 + (f) ) + ( (r) * 10 ) )
#define SQ64(sq120) (Sq120ToSq64[(sq120)])
#define SQ120(sq64) (Sq64ToSq120[(sq64)])
#define CLRBIT(bb,sq) ((bb) &= ClearMask[(sq)])
#define SETBIT(bb,sq) ((bb) |= SetMask[(sq)])
#define IsBQ(p) (PieceBishopQueen[(p)])
#define IsRQ(p) (PieceRookQueen[(p)])
#define IsKn(p) (PieceKnight[(p)])
#define IsKi(p) (PieceKing[(p)])
#define MIRROR64(sq) (Mirror64[(sq)])
#define COL(i) ((i) % 8)
#define ROW(i) ((i) / 8)
#define KN_DIR_COUNT 8
#define RK_DIR_COUNT 4
#define BI_DIR_COUNT 4
#define KI_DIR_COUNT 8

extern int 			Sq120ToSq64[120];
extern int 			Sq64ToSq120[64];
extern U64 			SetMask[64];
extern U64 			ClearMask[64];
extern U64 			PieceKeys[13][120];
extern U64 			SideKey;
extern U64 			CastleKeys[16];
extern char 		PceChar[];
extern char 		SideChar[];
extern char 		RankChar[];
extern char 		FileChar[];
extern int 			PieceBig[13];
extern int 			PieceMaj[13];
extern int 			PieceMin[13];
extern int 			PieceValMG[13];
extern int 			PieceValEG[13];
extern int 			PieceCol[13];
extern int 			PiecePawn[13];
extern int 			FilesBrd[120];
extern int 			RanksBrd[120];
extern int			PieceKnight[13];
extern int 			PieceKing[13];
extern int 			PieceRookQueen[13];
extern int 			PieceBishopQueen[13];
extern int 			PieceSlides[13];
extern int 			Mirror64[64];
extern U64 			FileBBMask[8];
extern U64 			RankBBMask[8];
extern U64 			BlackPassedMask[64];
extern U64 			WhitePassedMask[64];
extern U64 			IsolatedMask[64];
extern U64 			BlackConnectedMask[64];
extern U64 			WhiteConnectedMask[64];
extern U64 			BlackPawnShield[64];
extern U64 			WhitePawnShield[64];
extern S_HASHTABLE 	HashTable[1];
extern int 			DistTable[64][64];
extern const int 	KnDir[];
extern const int 	RkDir[];
extern const int 	BiDir[];
extern const int 	KiDir[];

extern void 		AllInit();
extern int 			PopBit(U64 *bb);
extern int 			CountBits(U64 b);
extern U64 			GeneratePosKey(const S_BOARD *pos);
extern void 		ResetBoard(S_BOARD *pos);
extern int 			ParseFen(char *fen, S_BOARD *pos);
extern void 		PrintBoard(const S_BOARD *pos);
extern void 		UpdateListsMaterial(S_BOARD *pos);
extern int 			SqAttacked(const int sq, const int side, const S_BOARD *pos);
extern char 		*PrMove(const int move);
extern char 		*PrSq(const int sq);
extern void 		PrintMoveList(const S_MOVELIST *list);
extern int 			ParseMove(char *ptrChar, S_BOARD *pos);
extern void 		DebugAnalysisTest(S_BOARD *pos, S_SEARCHINFO *info, S_HASHTABLE *table);
extern int 			GetMobility(const S_BOARD *pos, const int side);
extern void 		GenerateAllMoves(const S_BOARD *pos, S_MOVELIST *list);
extern void 		GenerateAllCaps(const S_BOARD *pos, S_MOVELIST *list);
extern int 			MoveExists(S_BOARD *pos, const int move);
extern void 		InitMvvLva();
extern int 			MakeMove(S_BOARD *pos, int move);
extern void 		TakeMove(S_BOARD *pos);
extern void 		MakeNullMove(S_BOARD *pos);
extern void 		TakeNullMove(S_BOARD *pos);
extern void 		SearchPosition(S_BOARD *pos, S_SEARCHINFO *info, S_HASHTABLE *table);
extern void 		InitSearch();
extern void 		CleanupThreads();
extern int 			GetTimeMs();
extern void 		ReadInput(S_SEARCHINFO *info);
extern void 		InitHashTable(S_HASHTABLE *table, const int MB);
extern void 		StoreHashEntry(S_BOARD *pos, S_HASHTABLE *table, const int move, int score, const int flags, const int depth);
extern int 			ProbeHashEntry(S_BOARD *pos, S_HASHTABLE *table, int *move, int *score, int alpha, int beta, int depth);
extern int 			ProbePvMove(const S_BOARD *pos, S_HASHTABLE *table);
extern int 			GetPvLine(const int depth, S_BOARD *pos, S_HASHTABLE *table);
extern void 		ClearHashTable(S_HASHTABLE *table);
extern int 			EvalPosition(S_BOARD *pos);
extern void 		InitEval();
extern void 		Uci_Loop(S_BOARD *pos, S_SEARCHINFO *info, S_HASHTABLE *table);
extern void 		TuneEval(S_BOARD *pos, char *fileIn, char *fileOut, char *fileLog, int use_tanh);

#endif
