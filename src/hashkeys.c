#include "stdio.h"
#include "defs.h"

// Prime number for additional mixing
#define PRIME_MIXER 0x100000001B3ULL

// Rotate left operation for 64-bit values
#define ROTL64(x, r) (((x) << (r)) | ((x) >> (64 - (r))))

static inline U64 MixHash(U64 h) {
    // FNV-1a inspired mixing function
    h ^= h >> 33;
    h *= PRIME_MIXER;
    h ^= h >> 33;
    h *= 0x9ddfea08eb382d69ULL;
    h ^= h >> 33;
    return h;
}

U64 GeneratePosKey(const S_BOARD *pos) {

	int sq = 0;
	U64 finalKey = 0;
	int piece = EMPTY;
	finalKey = 0x5851F42D4C957F2DULL;
	
	// pieces
	for(sq = 0; sq < 120; ++sq) {
		piece = pos->pieces[sq];
		if(piece!=NO_SQ && piece!=EMPTY && piece != OFFBOARD) {
			finalKey ^= ROTL64(PieceKeys[piece][sq], (piece * sq) & 0x3F);
		}		
	}
	
	// Side to move
	if(pos->side == WHITE) {
		finalKey ^= SideKey;
	}
		
	// En passant square
	if(pos->enPas != NO_SQ) {
		// Use a different mixing for en passant to avoid collisions
		finalKey ^= ROTL64(PieceKeys[EMPTY][pos->enPas], 17);
	}
	
	// Castle permissions
	finalKey ^= ROTL64(CastleKeys[pos->castlePerm], pos->castlePerm);
	
	// Material count as additional feature
	finalKey ^= ((U64)pos->material[WHITE] << 32) | (U64)pos->material[BLACK];
	
	// Fifty move counter to differentiate positions with same piece setup
	finalKey ^= (U64)pos->fiftyMove;
	
	// Final mixing
	finalKey = MixHash(finalKey);
	
	return finalKey;
}