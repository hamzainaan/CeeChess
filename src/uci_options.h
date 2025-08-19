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

#endif // UCI_OPTIONS_H