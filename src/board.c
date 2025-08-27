#include "stdio.h"
#include "defs.h"

int PceListOk(const S_BOARD *pos) {
	int pce = WHITE_PAWN;
	int sq;
	int num;
	for(pce = WHITE_PAWN; pce <= BLACK_KING; ++pce) {
		if(pos->pceNum[pce]<0 || pos->pceNum[pce]>=10) return 0;
	}

	if(pos->pceNum[WHITE_KING]!=1 || pos->pceNum[BLACK_KING]!=1) return 0;

	for(pce = WHITE_PAWN; pce <= BLACK_KING; ++pce) {
		for(num = 0; num < pos->pceNum[pce]; ++num) {
			sq = pos->pList[pce][num];
			if(!SqOnBoard(sq)) return 0;
		}
	}
    return 1;
}

void UpdateListsMaterial(S_BOARD *pos) {

	int piece,sq,index,colour;

	for(index = 0; index < 120; ++index) {
		sq = index;
		piece = pos->pieces[index];
		if(piece!=OFFBOARD && piece!= EMPTY) {
			colour = PieceCol[piece];

		    if( PieceBig[piece] == 1) pos->bigPce[colour]++;
		    if( PieceMin[piece] == 1) pos->minPce[colour]++;
		    if( PieceMaj[piece] == 1) pos->majPce[colour]++;

			pos->material[colour] += PieceValMG[piece];
			pos->material[colour + 2] += PieceValEG[piece];
			pos->pList[piece][pos->pceNum[piece]] = sq;
			pos->pceNum[piece]++;

			if(piece==WHITE_KING) pos->KingSq[WHITE] = sq;
			if(piece==BLACK_KING) pos->KingSq[BLACK] = sq;

			if(piece==WHITE_PAWN) {
				SETBIT(pos->pawns[WHITE],SQ64(sq));
				SETBIT(pos->pawns[BOTH],SQ64(sq));
			} else if(piece==BLACK_PAWN) {
				SETBIT(pos->pawns[BLACK],SQ64(sq));
				SETBIT(pos->pawns[BOTH],SQ64(sq));
			}
		}
	}
}

int ParseFen(char *fen, S_BOARD *pos) {
	int  rank = RANK_8;
    int  file = FILE_A;
    int  piece = 0;
    int  count = 0;
    int  i = 0;
	int  sq64 = 0;
	int  sq120 = 0;

	ResetBoard(pos);

	while ((rank >= RANK_1) && *fen) {
	    count = 1;
		switch (*fen) {
            case 'p': piece = BLACK_PAWN; break;
            case 'r': piece = BLACK_ROOK; break;
            case 'n': piece = BLACK_KNIGHT; break;
            case 'b': piece = BLACK_BISHOP; break;
            case 'k': piece = BLACK_KING; break;
            case 'q': piece = BLACK_QUEEN; break;
            case 'P': piece = WHITE_PAWN; break;
            case 'R': piece = WHITE_ROOK; break;
            case 'N': piece = WHITE_KNIGHT; break;
            case 'B': piece = WHITE_BISHOP; break;
            case 'K': piece = WHITE_KING; break;
            case 'Q': piece = WHITE_QUEEN; break;

            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
                piece = EMPTY;
                count = *fen - '0';
                break;

            case '/':
            case ' ':
                rank--;
                file = FILE_A;
                fen++;
                continue;

            default:
                printf("FEN error \n");
                return -1;
        }

		for (i = 0; i < count; i++) {
            sq64 = rank * 8 + file;
			sq120 = SQ120(sq64);
            if (piece != EMPTY) {
                pos->pieces[sq120] = piece;
            }
			file++;
        }
		fen++;
	}

	pos->side = (*fen == 'w') ? WHITE : BLACK;
	fen += 2;

	for (i = 0; i < 4; i++) {
        if (*fen == ' ') {
            break;
        }
		switch(*fen) {
			case 'K': pos->castlePerm |= WKCA; break;
			case 'Q': pos->castlePerm |= WQCA; break;
			case 'k': pos->castlePerm |= BKCA; break;
			case 'q': pos->castlePerm |= BQCA; break;
			default:	     break;
        }
		fen++;
	}
	fen++;

	if (*fen != '-') {
		file = fen[0] - 'a';
		rank = fen[1] - '1';
		pos->enPas = FR2SQ(file,rank);
    }

	pos->posKey = GeneratePosKey(pos);
	UpdateListsMaterial(pos);
	return 0;
}

void ResetBoard(S_BOARD *pos) {

	int index = 0;

	for(index = 0; index < 120; ++index) {
		pos->pieces[index] = OFFBOARD;
	}

	for(index = 0; index < 64; ++index) {
		pos->pieces[SQ120(index)] = EMPTY;
	}

	for(index = 0; index < 2; ++index) {
		pos->bigPce[index] = 0;
		pos->majPce[index] = 0;
		pos->minPce[index] = 0;
	}

	for(index = 0; index < 3; ++index) {
		pos->pawns[index] = 0ULL;
	}

	for(index = 0; index < 4; ++index) {
		pos->material[index] = 0;
	}

	for(index = 0; index < 13; ++index) {
		pos->pceNum[index] = 0;
	}

	pos->KingSq[WHITE] = pos->KingSq[BLACK] = NO_SQ;

	pos->side = BOTH;
	pos->enPas = NO_SQ;
	pos->fiftyMove = 0;

	pos->ply = 0;
	pos->hisPly = 0;

	pos->castlePerm = 0;

	pos->posKey = 0ULL;

}
void PrintBoard(const S_BOARD *pos) {

	int sq,file,rank,piece;

	printf("\nGame Board:\n\n");

	for(rank = RANK_8; rank >= RANK_1; rank--) {
		printf("%d  ",rank+1);
		for(file = FILE_A; file <= FILE_H; file++) {
			sq = FR2SQ(file,rank);
			piece = pos->pieces[sq];
			printf("%3c",PceChar[piece]);
		}
		printf("\n");
	}

	printf("\n   ");
	for(file = FILE_A; file <= FILE_H; file++) {
		printf("%3c",'a'+file);
	}
	printf("\n");
	printf("side:%c\n",SideChar[pos->side]);
	printf("enPas:%d\n",pos->enPas);
	printf("castle:%c%c%c%c\n",
			pos->castlePerm & WKCA ? 'K' : '-',
			pos->castlePerm & WQCA ? 'Q' : '-',
			pos->castlePerm & BKCA ? 'k' : '-',
			pos->castlePerm & BQCA ? 'q' : '-'
			);
	printf("PosKey:%llX\n",pos->posKey);
}