// uci.c

#include "stdio.h"
#include "defs.h"
#include "string.h"
#include "uci_options.h"
#include "config.h"

#define INPUTBUFFER 400 * 6

void ParseGo(char* line, S_SEARCHINFO *info, S_BOARD *pos, S_HASHTABLE *table) {

	int depth = -1, movestogo = 30, movetime = -1, time = -1, inc = 0;
    char *ptr = NULL;
	info->timeset = 0;

	if ((ptr = strstr(line,"infinite"))) {
		;
	}

	if ((ptr = strstr(line,"binc")) && pos->side == BLACK) {
		inc = atoi(ptr + 5);
	}

	if ((ptr = strstr(line,"winc")) && pos->side == WHITE) {
		inc = atoi(ptr + 5);
	}

	if ((ptr = strstr(line,"wtime")) && pos->side == WHITE) {
		time = atoi(ptr + 6);
	}

	if ((ptr = strstr(line,"btime")) && pos->side == BLACK) {
		time = atoi(ptr + 6);
	}

	if ((ptr = strstr(line,"movestogo"))) {
		movestogo = atoi(ptr + 10);
	}

	if ((ptr = strstr(line,"movetime"))) {
		movetime = atoi(ptr + 9);
	}

	if ((ptr = strstr(line,"depth"))) {
		depth = atoi(ptr + 6);
	}

	if(movetime != -1) {
		time = movetime;
		movestogo = 1;
	}

	info->starttime = GetTimeMs();
	info->depth = depth;

	if(time != -1) {
		info->timeset = 1;
		time /= movestogo;
		time -= 50;
		info->stoptime = info->starttime + time + inc;
	}

	if(depth == -1) {
		info->depth = MAXDEPTH;
	}

	SearchPosition(pos, info, table);
}

void ParsePosition(char* lineIn, S_BOARD *pos) {

	lineIn += 9;
    char *ptrChar = lineIn;

    if(strncmp(lineIn, "startpos", 8) == 0){
        ParseFen(START_FEN, pos);
    } else {
        ptrChar = strstr(lineIn, "fen");
        if(ptrChar == NULL) {
            ParseFen(START_FEN, pos);
        } else {
            ptrChar+=4;
            ParseFen(ptrChar, pos);
        }
    }

	ptrChar = strstr(lineIn, "moves");
	int move;

	if(ptrChar != NULL) {
        ptrChar += 6;
        while(*ptrChar) {
              move = ParseMove(ptrChar,pos);
			  if(move == 0) break;
			  MakeMove(pos, move);
              pos->ply=0;
              while(*ptrChar && *ptrChar!= ' ') ptrChar++;
              ptrChar++;
        }
    }
}

void Uci_Loop(S_BOARD *pos, S_SEARCHINFO *info, S_HASHTABLE *table) {

	setbuf(stdin, NULL);
    setbuf(stdout, NULL);

	// Initialize UCI options
	InitUciOptions();
	// Set hash table pointer for option handlers
	SetHashTablePtr(table);

	char line[INPUTBUFFER];
  	printf("id name %s\n",FULL_ENGINE_NAME);
  	printf("id author %s\n",ENGINE_AUTHOR);
	// Print all UCI options
	PrintUciOptions();
  	printf("uciok\n");

	while (1) {
		memset(&line[0], 0, sizeof(line));
        fflush(stdout);
        if (!fgets(line, INPUTBUFFER, stdin))
        continue;

        if (line[0] == '\n')
        continue;

        if (!strncmp(line, "isready", 7)) {
            printf("readyok\n");
            continue;
        } else if (!strncmp(line, "position", 8)) {
            ParsePosition(line, pos);
        } else if (!strncmp(line, "ucinewgame", 10)) {
			ClearHashTable(table);
            ParsePosition("position startpos\n", pos);
        } else if (!strncmp(line, "go", 2)) {
            ParseGo(line, info, pos, table);
        } else if (!strncmp(line, "run", 3)) {
            ParseFen(START_FEN, pos);
            ParseGo("go infinite", info, pos, table);
        } else if (!strncmp(line, "quit", 4)) {
            info->quit = 1;
            break;
        } else if (!strncmp(line, "uci", 3)) {
            printf("id name %s\n",FULL_ENGINE_NAME);
            printf("id author %s\n",ENGINE_AUTHOR);
            PrintUciOptions();
            printf("uciok\n");
        } else if (!strncmp(line, "debug", 4)) {
            DebugAnalysisTest(pos, info, table);
            break;
        } else if (!strncmp(line, "setoption name ", 15)) {
			char option_name[64] = "";
			char option_value[64] = "";
			char *name_start = line + 15;
			char *value_start = strstr(name_start, " value ");
			
			if (value_start != NULL) {
				// Extract option name
				int name_len = value_start - name_start;
				strncpy(option_name, name_start, name_len);
				option_name[name_len] = '\0';
				
				// Extract option value
				value_start += 7; // Skip " value "
				strcpy(option_value, value_start);
				
				// Process the option
				ProcessUciOption(option_name, option_value);
			}
		}
		if(info->quit) break;
    }
}