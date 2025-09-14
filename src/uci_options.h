#ifndef UCI_OPTIONS_H
#define UCI_OPTIONS_H

#include "defs.h"

// UCI option types
typedef enum {
    UCI_OPTION_SPIN,
    UCI_OPTION_CHECK,
    UCI_OPTION_COMBO,
    UCI_OPTION_BUTTON,
    UCI_OPTION_STRING
} UCI_OPTION_TYPE;

// Chess variant options
typedef enum {
    VARIANT_STANDARD,
    VARIANT_CHESS960,
    VARIANT_COUNT
} CHESS_VARIANT;

// Style options for time management
typedef enum {
    STYLE_NORMAL,
    STYLE_AGGRESSIVE,
    STYLE_SOLID,
    STYLE_COUNT
} PLAY_STYLE;

// UCI option structure
typedef struct {
    char name[64];
    UCI_OPTION_TYPE type;
    int default_value;
    int min_value;
    int max_value;
    int current_value;
    void (*on_change)(int);
} S_UCI_OPTION;

// UCI options array
extern S_UCI_OPTION UciOptions[];

// Number of UCI options
extern int UciOptionsCount;

// Initialize UCI options
void InitUciOptions();

// Process UCI option change
void ProcessUciOption(char* name, char* value);

// Print all UCI options
void PrintUciOptions();

// Set the hash table pointer for option handlers
void SetHashTablePtr(S_HASHTABLE *table);

// Get the current thread count
int GetThreadCount();

// Get the current play style
PLAY_STYLE GetPlayStyle();

// Get style name as string
const char* GetStyleName(PLAY_STYLE style);

// Get pondering status
int GetPonderingEnabled();

// Get chess variant
CHESS_VARIANT GetChessVariant();

// Get variant name as string
const char* GetVariantName(CHESS_VARIANT variant);

#endif // UCI_OPTIONS_H