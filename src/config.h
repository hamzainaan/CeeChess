#ifndef CONFIG_H
#define CONFIG_H

#define START_FEN  "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

// Engine information
#define ENGINE_NAME "CeeChess"
#define ENGINE_VERSION "1.5-dev"
#define ENGINE_AUTHOR "Bctboi23"

// Options
#define MAXDEPTH 64
#define INFINITE 30000
#define MAX_FEN_LEN 90
#define MAXGAMEMOVES 2048
#define MAXPOSITIONMOVES 256
#define ISMATE (INFINITE - MAXDEPTH)

// UCI options
#define DEFAULT_HASH_SIZE 256
#define MIN_HASH_SIZE 4
#define MAX_HASH_SIZE 65536  // 64 GB
#define DEFAULT_THREADS 1
#define MIN_THREADS 1
#define MAX_THREADS 128

// Full engine name with version
#define FULL_ENGINE_NAME ENGINE_NAME " " ENGINE_VERSION

#endif // CONFIG_H