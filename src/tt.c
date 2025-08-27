#include "stdio.h"
#include "defs.h"
#include "config.h"

#ifdef _MSC_VER
#include <xmmintrin.h> // _mm_prefetch
#include <malloc.h>    // _aligned_malloc
#else
#include <stdlib.h>    // aligned_alloc
#endif

S_HASHTABLE HashTable[1];

int GetPvLine(const int depth, S_BOARD *pos, S_HASHTABLE *table) {

	int move = ProbePvMove(pos, table);
	int count = 0;
	
	while(move != 0 && count < depth) {
		if( MoveExists(pos, move) ) {
			MakeMove(pos, move);
			pos->PvArray[count++] = move;
		} else {
			break;
		}		
		move = ProbePvMove(pos, table);	
	}
	
	while(pos->ply > 0) {
		TakeMove(pos);
	}
	return count;
}

void ClearHashTable(S_HASHTABLE *table) {

  S_HASHENTRY *tableEntry;
  
  for (tableEntry = table->pTable; tableEntry < table->pTable + table->numEntries; tableEntry++) {
    tableEntry->posKey = 0ULL;
    tableEntry->move = 0;
    tableEntry->depth = 0;
    tableEntry->score = 0;
    tableEntry->flags = 0;
	tableEntry->age = 0;
  }
  table->newWrite=0;
  table->currentage=0;
}

void InitHashTable(S_HASHTABLE *table, const int MB) {  
	
	size_t HashSize = (size_t)0x100000 * (size_t)MB;
    size_t rawEntries = HashSize / sizeof(S_HASHENTRY);
    
    // Round down to the nearest power of 2
    size_t powerOf2 = 1;
    while (powerOf2 * 2 <= rawEntries)
        powerOf2 *= 2;
    
    // Align with cache line size (typically 64 bytes)
    const size_t CACHE_LINE_SIZE = 64;
    size_t entrySize = sizeof(S_HASHENTRY);
    size_t entriesPerCacheLine = CACHE_LINE_SIZE / entrySize;
    
    if (entriesPerCacheLine > 1) {
        // Make hash table size a multiple of cache line size
        powerOf2 = (powerOf2 / entriesPerCacheLine) * entriesPerCacheLine;
    }
        
    table->numEntries = powerOf2;
	
	if(table->pTable!=NULL) {
		#ifdef _MSC_VER
		_aligned_free(table->pTable);
		#else
		free(table->pTable); // memory allocated with aligned_alloc can be freed with normal free
		#endif
	}
		
	#ifdef _MSC_VER
    table->pTable = (S_HASHENTRY *) _aligned_malloc(table->numEntries * sizeof(S_HASHENTRY), 64);
	#else
	// aligned_alloc may not be supported in MinGW, using standard malloc
	table->pTable = (S_HASHENTRY *) malloc(table->numEntries * sizeof(S_HASHENTRY));
	#endif
	if(table->pTable == NULL) {
		if(MB/2 >= MIN_HASH_SIZE) {
			InitHashTable(table, MB/2);
		} else {
			printf("Cannot allocate hash table with minimum size. Exiting.\n");
			exit(1);
		}
	} else {
		ClearHashTable(table);
	}
	
}

int ProbeHashEntry(S_BOARD *pos, S_HASHTABLE *table, int *move, int *score, int alpha, int beta, int depth) {

	size_t index = pos->posKey & (table->numEntries - 1); // Bit masking instead of modulo
	
	// Prefetch hash entry
	#ifdef _MSC_VER
	_mm_prefetch((const char*)&table->pTable[index], _MM_HINT_T0);
	#elif defined(__GNUC__)
	__builtin_prefetch(&table->pTable[index]);
	#endif
	
	S_HASHENTRY *pentry = &table->pTable[index];
	
	if(pentry->posKey == pos->posKey) {
		*move = pentry->move;
		// if hash depth > depth move score is usable
		if(pentry->depth >= depth){
			table->hit++;
			*score = pentry->score;

			if(*score > ISMATE) *score -= pos->ply;
            else if(*score < -ISMATE) *score += pos->ply;
			
			switch(pentry->flags) {
                case 1: if(*score<=alpha) {
                    *score=alpha;
                    return 1;
                    }
                    break;
                case 2: if(*score>=beta) {
                    *score=beta;
                    return 1;
                    }
                    break;
                case 3:
                    return 1;
                    break;
                default: break;
            }
		}
		// otherwise move order is usable
	}
	
	return 0;
}

void StoreHashEntry(S_BOARD *pos, S_HASHTABLE *table, const int move, int score, const int flags, const int depth) {

	size_t index = pos->posKey & (table->numEntries - 1); // Bit masking instead of modulo
	
	// Prefetch hash entry
	#ifdef _MSC_VER
	_mm_prefetch((const char*)&table->pTable[index], _MM_HINT_T0);
	#elif defined(__GNUC__)
	__builtin_prefetch(&table->pTable[index]);
	#endif
	
	S_HASHENTRY *pentry = &table->pTable[index];

	if(pentry->posKey == 0) {
		table->newWrite++;
	} else {
		if (!(pentry->age < table->currentage || pentry->depth < depth))
			return;
		table->overWrite++;
	}
	
	if(score > ISMATE) score += pos->ply;
    else if(score < -ISMATE) score -= pos->ply;
	
	pentry->move = move;
    pentry->posKey = pos->posKey;
	pentry->flags = flags;
	pentry->score = score;
	pentry->depth = depth;
	pentry->age = table->currentage;
}

int ProbePvMove(const S_BOARD *pos, S_HASHTABLE *table) {

	size_t index = pos->posKey & (table->numEntries - 1); // Bit masking instead of modulo
	
	// Prefetch hash entry
	#ifdef _MSC_VER
	_mm_prefetch((const char*)&table->pTable[index], _MM_HINT_T0);
	#elif defined(__GNUC__)
	__builtin_prefetch(&table->pTable[index]);
	#endif
	
	S_HASHENTRY *pentry = &table->pTable[index];
	
	if(pentry->posKey == pos->posKey) {
		return pentry->move;
	}
	
	return 0;
}