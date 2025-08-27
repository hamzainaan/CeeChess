#include <string.h>
#include <pthread.h>
#include "stdio.h"
#include "defs.h"
#include "config.h"
#include "math.h"
#include "uci_options.h"

// Null Move Pruning Values
static const int R = 2;
static const int minDepth = 3;

// Razoring Values
static const int RazorDepth = 2;
static const int RazorMargin[3] = {0, 200, 400};

// Futility Values
static const int FutilityDepth = 10;
static const int FutilityMargin = 150;

// Reverse Futility Values
static const int RevFutilityDepth = 10;
static const int RevFutilityMargin = 200;

// LMR Values
static const int LateMoveDepth = 3;
static const int FullSearchMoves = 2;
int LMRTable[64][64];

// Aspiration Window Values
static const int AspirationDepth = 4;
static const int AspirationDelta = 25;

// Probcut Values
static const int ProbcutDepth = 4;
static const int ProbcutMargin = 100;

// Singular Extensions Values
static const int SingularExtensionDepth = 6;
static const int SingularMargin = 50;

// Global variables for thread management
S_SEARCHINFO *ThreadInfo[MAX_THREADS];
S_BOARD ThreadBoards[MAX_THREADS];
pthread_mutex_t GlobalMutex;

void InitSearch() {
	// creating the LMR table entries (idea from Ethereal)
	for (int moveDepth = 1; moveDepth < 64; moveDepth++)
  		for (int played = 1; played < 64; played++)
      		LMRTable[moveDepth][played] = 1 + (log(moveDepth) * log(played) / 1.75);
	
	// Initialize global mutex
	pthread_mutex_init(&GlobalMutex, NULL);
	
	// Initialize thread info pointers to null
	for (int i = 0; i < MAX_THREADS; i++) {
		ThreadInfo[i] = NULL;
	}
}

// Clean up thread resources
void CleanupThreads() {
	// Destroy global mutex
	pthread_mutex_destroy(&GlobalMutex);
	
	// Free thread info structures and destroy their mutexes
	for (int i = 0; i < MAX_THREADS; i++) {
		if (ThreadInfo[i] != NULL) {
			pthread_mutex_destroy(&ThreadInfo[i]->mutex);
			free(ThreadInfo[i]);
			ThreadInfo[i] = NULL;
		}
	}
}

static void CheckUp(S_SEARCHINFO *info) {
	// .. check if time up, or interrupt from GUI
	if(info->timeset == 1 && GetTimeMs() > info->stoptime) {
		// Use mutex to safely update the stopped flag
		pthread_mutex_lock(&info->mutex);
		info->stopped = 1;
		pthread_mutex_unlock(&info->mutex);
	}

	// Only the main thread should read input
	if (info->threadNum == 0) {
		ReadInput(info);
	}
}

static void PickNextMove(int moveNum, S_MOVELIST *list) {

	S_MOVE temp;
	int index = 0;
	int bestScore = 0;
	int bestNum = moveNum;

	for (index = moveNum; index < list->count; ++index) {
		if (list->moves[index].score > bestScore) {
			bestScore = list->moves[index].score;
			bestNum = index;
		}
	}

	temp = list->moves[moveNum];
	list->moves[moveNum] = list->moves[bestNum];
	list->moves[bestNum] = temp;
}

static int IsRepetition(const S_BOARD *pos) {

	int index = 0;

	for(index = pos->hisPly - pos->fiftyMove; index < pos->hisPly-1; ++index) {
		if(pos->posKey == pos->history[index].posKey) {
			return 1;
		}
	}
	return 0;
}

static void ClearForSearch(S_BOARD *pos, S_SEARCHINFO *info, S_HASHTABLE *table) {

	int index = 0;
	int index2 = 0;

	for(index = 0; index < 13; ++index) {
		for(index2 = 0; index2 < 120; ++index2) {
			pos->searchHistory[index][index2] = 0;
		}
	}

	for(index = 0; index < 2; ++index) {
		for(index2 = 0; index2 < MAXDEPTH; ++index2) {
			pos->searchKillers[index][index2] = 0;
		}
	}

	// Only the main thread should reset these values
	if (info->threadNum == 0) {
		table->overWrite=0;
		table->hit=0;
		table->cut=0;
		table->currentage++;
	}

	pos->ply = 0;

	// Use mutex to safely update info fields
	pthread_mutex_lock(&info->mutex);
	info->stopped = 0;
	info->nodes = 0;
	info->fh = 0;
	info->fhf = 0;
	info->singularExt = 0;
	info->searching = 0;
	pthread_mutex_unlock(&info->mutex);
}

static int Quiescence(int alpha, int beta, S_BOARD *pos, S_SEARCHINFO *info) {

	if(( info->nodes & 2047 ) == 0) {
		CheckUp(info);
	}

	// Thread-safe node counter increment
	pthread_mutex_lock(&info->mutex);
	info->nodes++;
	pthread_mutex_unlock(&info->mutex);

	if(IsRepetition(pos) || pos->fiftyMove >= 100) {
		return 0;
	}

	if(pos->ply > MAXDEPTH - 1) {
		return EvalPosition(pos);
	}

	// Mate Distance Pruning
	alpha = MAX(alpha, -INFINITE + pos->ply);
	beta = MIN(beta, INFINITE - pos->ply);
	if (alpha >= beta) {
		return alpha;
	}

	int Score = EvalPosition(pos);

	if(Score >= beta) {
		return beta;
	}

	if (Score > alpha) {
		alpha = Score;
	}

	S_MOVELIST list[1];
    GenerateAllCaps(pos,list);

    int MoveNum = 0;
	int Legal = 0;
	Score = -INFINITE;

	for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {

		// return immediately if stopped
		if(info->stopped == 1) {
			return beta;
		}

		PickNextMove(MoveNum, list);

        if ( !MakeMove(pos,list->moves[MoveNum].move))  {
            continue;
        }

		Legal++;
		Score = -Quiescence( -beta, -alpha, pos, info);
        TakeMove(pos);

		if(Score > alpha) {
			if(Score >= beta) {
				// Thread-safe counter updates
				pthread_mutex_lock(&info->mutex);
				if(Legal==1) {
					info->fhf++;
				}
				info->fh++;
				pthread_mutex_unlock(&info->mutex);
				return beta;
			}
			alpha = Score;
		}
    }
	return alpha;
}

// Forward declaration for AlphaBeta
static int AlphaBeta(int alpha, int beta, int depth, S_BOARD *pos, S_SEARCHINFO *info, int DoNull, int DoLMR, S_HASHTABLE *table);

static int IsSingular(int move, int depth, S_BOARD *pos, S_SEARCHINFO *info, S_HASHTABLE *table) {
	// Check if a move is singular (much better than all other moves)
	int score;
	int ttMove = 0;
	int ttScore = 0;
	int ttDepth = 0;
	int bound = -INFINITE;

	// Get the transposition table entry
	int found = ProbeHashEntry(pos, table, &ttMove, &ttScore, -INFINITE, INFINITE, 0);
	
	// If the move is not the hash move, it's not singular
	if (!found || ttMove != move) {
		return 0;
	}

	// Get the depth from the hash entry
	ttDepth = table->pTable[pos->posKey % table->numEntries].depth;

	// If the depth is too low, it's not singular
	if (ttDepth < depth - 3) {
		return 0;
	}

	// Set a reduced beta for the search
	int reducedBeta = ttScore - SingularMargin;

	// Search with a null window around the reduced beta
	S_MOVELIST list[1];
	GenerateAllMoves(pos, list);

	for (int i = 0; i < list->count; ++i) {
		if (list->moves[i].move == move) {
			continue; // Skip the move we're testing for singularity
		}

		if (!MakeMove(pos, list->moves[i].move)) {
			continue;
		}

		// Search with reduced depth and a null window
		score = -AlphaBeta(-reducedBeta-1, -reducedBeta, depth - 3, pos, info, 1, 1, table);

		TakeMove(pos);

		// If search was stopped, return 0
		if (info->stopped == 1) {
			return 0;
		}

		// If any move beats the reduced beta, the move is not singular
		if (score >= reducedBeta) {
			return 0;
		}

		// Keep track of the best alternative move
		if (score > bound) {
			bound = score;
		}
	}

	// If all moves failed low by a significant margin, the move is singular
	pthread_mutex_lock(&info->mutex);
	info->singularExt++;
	pthread_mutex_unlock(&info->mutex);
	return 1;
}

static int AlphaBeta(int alpha, int beta, int depth, S_BOARD *pos, S_SEARCHINFO *info, int DoNull, int DoLMR, S_HASHTABLE *table) {

	int InCheck = SqAttacked(pos->KingSq[pos->side],pos->side^1,pos);

	// Check Extension (Extend all checks before dropping into Quiescence)
	if(InCheck) {
		depth++;
	}

	if(depth <= 0) {
		return Quiescence(alpha, beta, pos, info);
		// return EvalPosition(pos);
	}

	if(( info->nodes & 2047 ) == 0) {
		CheckUp(info);
	}

	// Thread-safe node counter increment
	pthread_mutex_lock(&info->mutex);
	info->nodes++;
	pthread_mutex_unlock(&info->mutex);

	if((IsRepetition(pos) || pos->fiftyMove >= 100) && pos->ply) {
		return 0;
	}

	if(pos->ply > MAXDEPTH - 1) {
		return EvalPosition(pos);
	}

	// Mate Distance Pruning (finds mates more quickly)
	alpha = MAX(alpha, -INFINITE + pos->ply);
	beta = MIN(beta, INFINITE - pos->ply);
	if (alpha >= beta) {
		return alpha;
	}

	int Score = -INFINITE;
	int PvMove = 0;

	if( ProbeHashEntry(pos, table, &PvMove, &Score, alpha, beta, depth) == 1 ) {
		table->cut++;
		return Score;
	}

	// Internal Iterative Deepening (IID)
	// If we don't have a PV move and depth is sufficient, do a shallower search to find a good move
	if (PvMove == 0 && depth >= 4 && !InCheck) {
		Score = AlphaBeta(alpha, beta, depth - 2, pos, info, DoNull, DoLMR, table);
		PvMove = ProbePvMove(pos, table);
	}

	// Singular Extensions
	// If we have a PV move and depth is sufficient, check if it's singular (much better than all other moves)
	if (PvMove != 0 && depth >= SingularExtensionDepth && !InCheck && abs(beta) < ISMATE) {
		// Check if the move is singular
		if (IsSingular(PvMove, depth, pos, info, table)) {
			// Extend the search depth for this move
			depth++;
		}
	}

	int positionEval = EvalPosition(pos);

	// Razoring (prunes near alpha)
	if (depth <= RazorDepth && !PvMove && !InCheck && positionEval + RazorMargin[depth] <= alpha) {
		// drop into qSearch if move most likely won't beat alpha
		Score = Quiescence(alpha - RazorMargin[depth], beta + RazorMargin[depth], pos, info);
		if (Score + RazorMargin[depth] <= alpha) {
			return Score + RazorMargin[depth];
		}
	}

	// Reverse Futility Pruning (prunes near beta)
	if (depth <= RevFutilityDepth && !PvMove && !InCheck && abs(beta) < ISMATE && positionEval - (RevFutilityMargin * depth) >= beta) {
		return positionEval - (RevFutilityMargin * depth);
	}

	// Probcut: Try a reduced depth search with a raised beta to quickly detect strong moves
	if (depth >= ProbcutDepth && !InCheck && abs(beta) < ISMATE) {
		int probBeta = beta + ProbcutMargin;
		S_MOVELIST list[1];
		GenerateAllCaps(pos, list);

		for (int MoveNum = 0; MoveNum < list->count; ++MoveNum) {
			PickNextMove(MoveNum, list);

			if (!MakeMove(pos, list->moves[MoveNum].move)) {
				continue;
			}

			// Try a reduced depth search with raised beta
			int probScore = -AlphaBeta(-probBeta, -probBeta + 1, depth - 4, pos, info, 0, 0, table);
			TakeMove(pos);

			if (info->stopped == 1) {
				return beta;
			}

			// If this move beats the raised beta, it will likely beat the real beta
			if (probScore >= probBeta) {
				return probScore;
			}
		}
	}

	// Null Move Pruning
	if(depth >= minDepth && DoNull && !InCheck && pos->ply && (pos->bigPce[pos->side] > 0) && positionEval >= beta) {
		MakeNullMove(pos);
		Score = -AlphaBeta( -beta, -beta + 1, depth - 1 - R, pos, info, 0, 0, table);
		TakeNullMove(pos);
		if(info->stopped == 1) {
			return beta;
		}

		if (Score >= beta && abs(Score) < ISMATE) {
			pthread_mutex_lock(&info->mutex);
			info->nullCut++;
			pthread_mutex_unlock(&info->mutex);
			return beta;
		}
	}

	S_MOVELIST list[1];
  	GenerateAllMoves(pos,list);

  	int MoveNum = 0;
	int Legal = 0;
	int OldAlpha = alpha;
	int BestMove = 0;

	int BestScore = -INFINITE;

	Score = -INFINITE;

	if( PvMove != 0) {
		for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {
			if( list->moves[MoveNum].move == PvMove) {
				list->moves[MoveNum].score = 2000000;
				//printf("Pv move found \n");
				break;
			}
		}
	}

	int FoundPv = 0;

	// Futility Pruning flag (if node is futile (unlikely to raise alpha), this flag is set)
	int FutileNode = (depth <= FutilityDepth && positionEval + (FutilityMargin * depth) <= alpha && abs(Score) < ISMATE && (pos->bigPce[pos->side] > 0)) ? 1 : 0;

	for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {

		// return if stopped
		if(info->stopped == 1) {
			return beta;
		}

		PickNextMove(MoveNum, list);

		// Futility Pruning (if node is considered futile, and at least 1 legal move has been searched, don't search any more quiet moves in the position)
		int isMoveCheck = SqAttacked(pos->KingSq[pos->side^1],pos->side,pos);
		int nonCapture = !(list->moves[MoveNum].move & 0x7C000);
		int isQuiet = (nonCapture && !(list->moves[MoveNum].move & 0xF00000) && !isMoveCheck);
		if (Legal && FutileNode && isQuiet) {
			continue;
		}

		// if move is legal, play it
		if ( !MakeMove(pos,list->moves[MoveNum].move))  {
			continue;
		}

		Legal++;

		// PVS (speeds up search with good move ordering)
		if (FoundPv == 1) {

			// Late Move Reductions (reduces quiet moves late in the search)
			if (depth >= LateMoveDepth && Legal > FullSearchMoves && !InCheck && isQuiet && DoLMR) {

				// get initial reduction depth
				int reduce = LMRTable[MIN(depth, 63)][MIN(Legal, 63)];

				// reduce less for killer moves
				if ((list->moves[MoveNum].move == pos->searchKillers[0][pos->ply]) || 
					(list->moves[MoveNum].move == pos->searchKillers[1][pos->ply])
				) reduce--;

				// do not fall directly into quiescence search
				reduce = MIN(depth - 1, MAX(reduce, 1));

				// print reduction depth at move number
				// printf("reduction: %d depth: %d moveNum: %d\n", (reduce - 1), depth, Legal);

				// search with the reduced depth
				Score = -AlphaBeta( -alpha - 1, -alpha, depth - reduce, pos, info, 1, 0, table);

				// If the LMR fails, do a full depth null-window search
				if (Score > alpha && Score < beta) {
					Score = -AlphaBeta( -alpha - 1, -alpha, depth - 1, pos, info, 1, 0, table);
				}
			} else {
				// If LMR conditions not met, do a null window search (because we are using PVS)
				Score = -AlphaBeta( -alpha - 1, -alpha, depth - 1, pos, info, 1, DoLMR, table);
			}
			// If the null window fails, do a full window search
			if (Score > alpha && Score < beta) {
				Score = -AlphaBeta( -beta, -alpha, depth - 1, pos, info, 1, 0, table);
			}

		} else {
			// If no PV found, do a full search
			Score = -AlphaBeta( -beta, -alpha, depth - 1, pos, info, 1, 0, table);
		}

		TakeMove(pos);

		if(Score > BestScore) {
			BestScore = Score;
			BestMove = list->moves[MoveNum].move;
		}
		if(Score > alpha) {
			if(Score >= beta) {
				// Thread-safe counter updates
				pthread_mutex_lock(&info->mutex);
				if(Legal==1) {
					info->fhf++;
				}
				info->fh++;
				pthread_mutex_unlock(&info->mutex);
				
				if (nonCapture) {
					if ((pos->searchKillers[0][pos->ply] != list->moves[MoveNum].move)) {
						pos->searchKillers[1][pos->ply] = pos->searchKillers[0][pos->ply];
						pos->searchKillers[0][pos->ply] = list->moves[MoveNum].move;
					}
				}
				StoreHashEntry(pos, table, BestMove, beta, 2, depth);
				return beta;
			}
			FoundPv = 1;
			alpha = Score;
			if (nonCapture) {
				pos->searchHistory[pos->pieces[FROMSQ(BestMove)]][TOSQ(BestMove)] += depth;
			}
		}
    }

	if(Legal == 0) {
		return InCheck * (-INFINITE + pos->ply);
	}

	if(alpha != OldAlpha) {
		StoreHashEntry(pos, table, BestMove, BestScore, 3, depth);
	} else {
		StoreHashEntry(pos, table, BestMove, alpha, 1, depth);
	}

	return alpha;
}

// Thread function for worker threads
void *ThreadStart(void *threadArgs) {
    S_SEARCHINFO *info = (S_SEARCHINFO *)threadArgs;
    S_BOARD *pos = &ThreadBoards[info->threadNum];
    S_HASHTABLE *table = HashTable;
    int depth = info->depth;
    
    // Mark thread as searching
    pthread_mutex_lock(&info->mutex);
    info->searching = 1;
    pthread_mutex_unlock(&info->mutex);
    
    // Start search at depth 1 and continue until stopped
    for (int currentDepth = 1; currentDepth <= depth; currentDepth++) {
        // Check if search should stop
        int stopped;
        pthread_mutex_lock(&info->mutex);
        stopped = info->stopped;
        pthread_mutex_unlock(&info->mutex);
        
        if (stopped) {
            break;
        }
        
        // Perform search with full window
        AlphaBeta(-INFINITE, INFINITE, currentDepth, pos, info, 1, 1, table);
    }
    
    // Mark thread as no longer searching
    pthread_mutex_lock(&info->mutex);
    info->searching = 0;
    pthread_mutex_unlock(&info->mutex);
    
    return NULL;
}

// Initialize a thread info structure
void InitThreadInfo(S_SEARCHINFO *info, int threadNum, int depth, int timeset, int starttime, int stoptime, int movestogo) {
    info->threadNum = threadNum;
    info->depth = depth;
    info->timeset = timeset;
    info->starttime = starttime;
    info->stoptime = stoptime;
    info->movestogo = movestogo;
    info->stopped = 0;
    info->quit = 0;
    info->nodes = 0;
    info->fh = 0;
    info->fhf = 0;
    info->nullCut = 0;
    info->singularExt = 0;
    info->threadCount = GetThreadCount();
    info->searching = 0;
    
    // Initialize mutex for this thread
    pthread_mutex_init(&info->mutex, NULL);
}

// Copy a board position to another board
void CopyBoard(S_BOARD *dest, S_BOARD *src) {
    memcpy(dest, src, sizeof(S_BOARD));
}

void SearchPosition(S_BOARD *pos, S_SEARCHINFO *info, S_HASHTABLE *table) {
    int bestMove = 0;
    int bestScore = -INFINITE;
    int currentDepth = 0, pvMoves = 0, pvNum = 0;
    U64 nps = 0;
    int threadCount = GetThreadCount();
    
    // Initialize main thread info
    info->threadNum = 0;
    info->threadCount = threadCount;
    pthread_mutex_init(&info->mutex, NULL);
    
    ClearForSearch(pos, info, table);
    
    // If using multiple threads, create and start worker threads
    if (threadCount > 1) {
        // Allocate and initialize thread info structures
        for (int i = 1; i < threadCount; i++) {
            // Allocate thread info if not already allocated
            if (ThreadInfo[i] == NULL) {
                ThreadInfo[i] = (S_SEARCHINFO *)malloc(sizeof(S_SEARCHINFO));
            }
            
            // Initialize thread info
            InitThreadInfo(ThreadInfo[i], i, info->depth, info->timeset, 
                          info->starttime, info->stoptime, info->movestogo);
            
            // Copy the board position to the thread's board
            CopyBoard(&ThreadBoards[i], pos);
            
            // Start the thread
            pthread_create(&ThreadInfo[i]->threadHandle, NULL, ThreadStart, ThreadInfo[i]);
        }
    }
    
    // Main thread search loop
    for(currentDepth = 1; currentDepth <= info->depth; ++currentDepth) {
        // Use Aspiration Windows for deeper searches
        if (currentDepth >= AspirationDepth) {
            int alpha = bestScore - AspirationDelta;
            int beta = bestScore + AspirationDelta;
            int failCount = 0;
            
            // Start with a narrow window around previous best score
            while (1) {
                bestScore = AlphaBeta(alpha, beta, currentDepth, pos, info, 1, 1, table);
                
                // If we get a score within our window, we're done
                if (bestScore > alpha && bestScore < beta) {
                    break;
                }
                
                // If search was stopped, exit
                pthread_mutex_lock(&info->mutex);
                int stopped = info->stopped;
                pthread_mutex_unlock(&info->mutex);
                
                if (stopped) {
                    break;
                }
                
                // If score fails low, adjust alpha and retry
                if (bestScore <= alpha) {
                    beta = (alpha + beta) / 2;
                    alpha = alpha - (AspirationDelta * (1 << failCount));
                    failCount++;
                }
                // If score fails high, adjust beta and retry
                else if (bestScore >= beta) {
                    beta = beta + (AspirationDelta * (1 << failCount));
                    failCount++;
                }
                
                // If we've failed too many times, do a full window search
                if (failCount >= 4) {
                    bestScore = AlphaBeta(-INFINITE, INFINITE, currentDepth, pos, info, 1, 1, table);
                    break;
                }
            }
        } else {
            // For shallow depths, use full window
            bestScore = AlphaBeta(-INFINITE, INFINITE, currentDepth, pos, info, 1, 1, table);
        }
        
        pthread_mutex_lock(&info->mutex);
        int stopped = info->stopped;
        pthread_mutex_unlock(&info->mutex);
        
        if(stopped) {
            break;
        }

        pvMoves = GetPvLine(currentDepth, pos, table);
        bestMove = pos->PvArray[0];

        int time = GetTimeMs() - info->starttime;
        
        // Calculate total nodes across all threads
        long totalNodes = info->nodes;
        if (threadCount > 1) {
            pthread_mutex_lock(&GlobalMutex);
            for (int i = 1; i < threadCount; i++) {
                if (ThreadInfo[i] != NULL) {
                    pthread_mutex_lock(&ThreadInfo[i]->mutex);
                    totalNodes += ThreadInfo[i]->nodes;
                    pthread_mutex_unlock(&ThreadInfo[i]->mutex);
                }
            }
            pthread_mutex_unlock(&GlobalMutex);
        }
        
        // Calculate nodes per second
        if (time > 0) {
            nps = ((U64)totalNodes * 1000ULL) / (U64)time;
        }
        
        if(abs(bestScore) > ISMATE) {
            bestScore = (bestScore > 0 ? INFINITE - bestScore + 1 : -INFINITE - bestScore) / 2;
            printf("info score mate %d depth %d nodes %ld nps %lld time %d ", bestScore, currentDepth, totalNodes, nps, time);
        } else {
            printf("info score cp %d depth %d nodes %ld nps %lld time %d ", bestScore, currentDepth, totalNodes, nps, time);
        }

        printf("pv");
        for(pvNum = 0; pvNum < pvMoves; ++pvNum) {
            printf(" %s", PrMove(pos->PvArray[pvNum]));
        }
        printf("\n");
    }
    
    // Stop all worker threads
    if (threadCount > 1) {
        // Set stopped flag for all threads
        pthread_mutex_lock(&info->mutex);
        info->stopped = 1;
        pthread_mutex_unlock(&info->mutex);
        
        // Wait for all threads to finish
        for (int i = 1; i < threadCount; i++) {
            if (ThreadInfo[i] != NULL) {
                pthread_mutex_lock(&ThreadInfo[i]->mutex);
                ThreadInfo[i]->stopped = 1;
                pthread_mutex_unlock(&ThreadInfo[i]->mutex);
                
                // Wait for thread to finish if it's still searching
                int searching;
                do {
                    pthread_mutex_lock(&ThreadInfo[i]->mutex);
                    searching = ThreadInfo[i]->searching;
                    pthread_mutex_unlock(&ThreadInfo[i]->mutex);
                    
                    if (searching) {
                        // Small sleep to avoid busy waiting
                        struct timespec ts;
                        ts.tv_sec = 0;
                        ts.tv_nsec = 1000000; // 1ms
                        nanosleep(&ts, NULL);
                    }
                } while (searching);
                
                // Join the thread
                pthread_join(ThreadInfo[i]->threadHandle, NULL);
            }
        }
    }
    
    printf("bestmove %s\n", PrMove(bestMove));
}