#include "stdio.h"
#include "defs.h"
#include "string.h"
#include "uci_options.h"
#include "config.h"

#define INPUTBUFFER 400 * 6

void ParseGo(char* line, S_SEARCHINFO *info, S_BOARD *pos, S_HASHTABLE *table) {

	int depth = -1, movestogo = 30, movetime = -1, time = -1, opptime = -1, inc = 0;
    char *ptr = NULL;
	info->timeset = 0;
	info->pondering = 0; // Reset pondering flag

	if ((ptr = strstr(line,"infinite"))) {
		depth = MAXDEPTH;
	}
	
	// Check if this is a ponder search
	if ((ptr = strstr(line,"ponder"))) {
		info->pondering = GetPonderingEnabled();
	}

	if ((ptr = strstr(line,"binc")) && pos->side == BLACK) {
		inc = atoi(ptr + 5);
	}

	if ((ptr = strstr(line,"winc")) && pos->side == WHITE) {
		inc = atoi(ptr + 5);
	}

	if ((ptr = strstr(line,"wtime"))) {
		if (pos->side == WHITE) {
			time = atoi(ptr + 6);
		} else {
			opptime = atoi(ptr + 6);
		}
	}

	if ((ptr = strstr(line,"btime"))) {
		if (pos->side == BLACK) {
			time = atoi(ptr + 6);
		} else {
			opptime = atoi(ptr + 6);
		}
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
		
		// Get current play style
		PLAY_STYLE style = GetPlayStyle();
		
		// Calculate base time allocation
		int timeAllocation = time / movestogo;
		
		// Adjust time based on style and position evaluation
		int eval = EvalPosition(pos);
		int positionAdvantage = (pos->side == WHITE) ? eval : -eval;
		
		// Time adjustment factor
		float timeFactor = 1.0f;
		
		switch (style) {
			case STYLE_AGGRESSIVE:
				// In aggressive mode:
				// - If opponent has less time and our position is not bad, play faster
				// - If we have less time and our position is good, play slower
				if (opptime != -1 && opptime < time && positionAdvantage >= -50) {
					// Opponent has less time and our position is not bad, play faster
					timeFactor = 0.7f;
				} else if (opptime != -1 && time < opptime && positionAdvantage > 50) {
					// We have less time but our position is good, play slower
					timeFactor = 1.3f;
				}
				break;
				
			case STYLE_SOLID:
				// In solid mode, always play calmly with consistent time usage
				timeFactor = 1.1f;
				break;
				
			case STYLE_NORMAL:
				// In normal mode, adjust time dynamically based on position
				if (positionAdvantage > 100) {
					// We have a clear advantage, play faster
					timeFactor = 0.8f;
				} else if (positionAdvantage < -100) {
					// We are at a disadvantage, think more
					timeFactor = 1.2f;
				}
				break;
				
			default:
				break;
		}
		
		// Apply the time factor
		timeAllocation = (int)(timeAllocation * timeFactor);
		
		// Safety margin
		timeAllocation -= 50;
		
		// Set the stop time
		info->stoptime = info->starttime + timeAllocation + inc;
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

// Forward declaration for TuneEval function
extern void TuneEval(S_BOARD *pos, char *fileIn, char *fileOut, char *fileLog, int use_tanh);

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
            // If we're pondering, stop the search
            if (info->pondering) {
                info->stopped = 1;
                // Wait for search to stop
                while (info->searching) {
                    // Small sleep to avoid busy waiting
                    struct timespec ts;
                    ts.tv_sec = 0;
                    ts.tv_nsec = 1000000; // 1ms
                    nanosleep(&ts, NULL);
                }
            }
            ParsePosition(line, pos);
        } else if (!strncmp(line, "ucinewgame", 10)) {
			ClearHashTable(table);
            ParsePosition("position startpos\n", pos);
        } else if (!strncmp(line, "go", 2)) {
            // If we're pondering, stop the search
            if (info->pondering) {
                info->stopped = 1;
                // Wait for search to stop
                while (info->searching) {
                    // Small sleep to avoid busy waiting
                    struct timespec ts;
                    ts.tv_sec = 0;
                    ts.tv_nsec = 1000000; // 1ms
                    nanosleep(&ts, NULL);
                }
            }
            ParseGo(line, info, pos, table);
        } else if (!strncmp(line, "run", 3)) {
            ParseFen(START_FEN, pos);
            ParseGo("go infinite", info, pos, table);
        } else if (!strncmp(line, "quit", 4)) {
            // If we're pondering, stop the search
            if (info->pondering) {
                info->stopped = 1;
                // Wait for search to stop
                while (info->searching) {
                    // Small sleep to avoid busy waiting
                    struct timespec ts;
                    ts.tv_sec = 0;
                    ts.tv_nsec = 1000000; // 1ms
                    nanosleep(&ts, NULL);
                }
            }
            info->quit = 1;
            break;
        } else if (!strncmp(line, "uci", 3)) {
            printf("id name %s\n",FULL_ENGINE_NAME);
            printf("id author %s\n",ENGINE_AUTHOR);
            PrintUciOptions();
            printf("uciok\n");
        } else if (!strncmp(line, "debug", 5)) {
            // If we're pondering, stop the search
            if (info->pondering) {
                info->stopped = 1;
                // Wait for search to stop
                while (info->searching) {
                    // Small sleep to avoid busy waiting
                    struct timespec ts;
                    ts.tv_sec = 0;
                    ts.tv_nsec = 1000000; // 1ms
                    nanosleep(&ts, NULL);
                }
            }
            DebugAnalysisTest(pos, info, table);
            break;
        } else if (!strncmp(line, "board", 5)) {
            PrintBoard(pos);
        } else if (!strncmp(line, "tune", 4)) {
            // If we're pondering, stop the search
            if (info->pondering) {
                info->stopped = 1;
                // Wait for search to stop
                while (info->searching) {
                    // Small sleep to avoid busy waiting
                    struct timespec ts;
                    ts.tv_sec = 0;
                    ts.tv_nsec = 1000000; // 1ms
                    nanosleep(&ts, NULL);
                }
            }
            
            // Parse tune command
            char *ptr = line + 5; // Skip "tune "
            int tune_option = atoi(ptr);
            
            printf("Starting tuning process with option %d...\n", tune_option);
            
            // Set file paths relative to the engine's directory
            char fileIn[] = "data.txt";
            char fileOut[] = "out.txt";
            char fileLog[] = "log.txt";
            
            // Call TuneEval with appropriate parameters based on tune option
            if (tune_option == 0) {
                // tune 0: Use sigmoid function
                printf("Using sigmoid function for tuning\n");
                TuneEval(pos, fileIn, fileOut, fileLog, 0);
            } else if (tune_option == 1) {
                // tune 1: Use tanh function
                printf("Using tanh function for tuning\n");
                TuneEval(pos, fileIn, fileOut, fileLog, 1);
            } else {
                printf("Invalid tune option. Use 'tune 0' for sigmoid or 'tune 1' for tanh\n");
            }
            
            printf("Tuning process completed\n");
        } else if (!strncmp(line, "ponderhit", 9)) {
            // Ponderhit command - our ponder was successful
            if (info->pondering) {
                // Convert from pondering to normal search
                info->pondering = 0;
                printf("info string Ponderhit received\n");
            }
        } else if (!strncmp(line, "setoption name ", 15)) {
			// If we're pondering, stop the search
            if (info->pondering) {
                info->stopped = 1;
                // Wait for search to stop
                while (info->searching) {
                    // Small sleep to avoid busy waiting
                    struct timespec ts;
                    ts.tv_sec = 0;
                    ts.tv_nsec = 1000000; // 1ms
                    nanosleep(&ts, NULL);
                }
            }
            
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
			} else {
				// For button type options (no value needed)
				// Just copy the name and pass empty value
				strcpy(option_name, name_start);
				// Remove any trailing whitespace or newline
				char *end = option_name + strlen(option_name) - 1;
				while (end > option_name && (*end == ' ' || *end == '\n' || *end == '\r')) {
					*end = '\0';
					end--;
				}
				
				// Process the button option
				ProcessUciOption(option_name, "");
			}
		}
		if(info->quit) break;
    }
}