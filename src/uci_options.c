#include "stdio.h"
#include "string.h"
#include "uci_options.h"
#include "config.h"
#include "defs.h"

// Forward declarations for option handlers
void HashOptionChanged(int value);
void ClearHashOptionPressed(int value);
void ThreadsOptionChanged(int value);
void StyleOptionChanged(int value);

// Global variable to store play style
PLAY_STYLE CurrentPlayStyle = STYLE_NORMAL;

// Style names
const char* StyleNames[STYLE_COUNT] = {
    "Normal",
    "Aggressive",
    "Solid"
};

// UCI options array
S_UCI_OPTION UciOptions[] = {
    {
        "Threads",
        UCI_OPTION_SPIN,
        DEFAULT_THREADS,
        MIN_THREADS,
        MAX_THREADS,
        DEFAULT_THREADS,
        ThreadsOptionChanged
    },
    {
        "Hash",
        UCI_OPTION_SPIN,
        DEFAULT_HASH_SIZE,
        MIN_HASH_SIZE,
        MAX_HASH_SIZE,
        DEFAULT_HASH_SIZE,
        HashOptionChanged
    },
    {
        "Clear Hash",
        UCI_OPTION_BUTTON,
        0,
        0,
        0,
        0,
        ClearHashOptionPressed
    },
    {
        "Style",
        UCI_OPTION_COMBO,
        STYLE_NORMAL,
        0,
        STYLE_COUNT - 1,
        STYLE_NORMAL,
        StyleOptionChanged
    }
};

// Number of UCI options
int UciOptionsCount = sizeof(UciOptions) / sizeof(S_UCI_OPTION);

S_HASHTABLE *HashTablePtr = NULL;

// Set the hash table pointer
void SetHashTablePtr(S_HASHTABLE *table) {
    HashTablePtr = table;
}

// Initialize UCI options
void InitUciOptions() {
    // Reset all options to their default values
    for (int i = 0; i < UciOptionsCount; i++) {
        UciOptions[i].current_value = UciOptions[i].default_value;
    }
}

// Process UCI option change
void ProcessUciOption(char* name, char* value) {
    for (int i = 0; i < UciOptionsCount; i++) {
        if (strcmp(UciOptions[i].name, name) == 0) {
            int intValue = 0;
            
            // Handle combo box for Style option
            if (UciOptions[i].type == UCI_OPTION_COMBO && strcmp(name, "Style") == 0) {
                // Convert style name to enum value
                for (int j = 0; j < STYLE_COUNT; j++) {
                    if (strcmp(value, StyleNames[j]) == 0) {
                        intValue = j;
                        break;
                    }
                }
            } else {
                // For other options, convert to integer
                intValue = atoi(value);
            }
            
            // Validate value based on option type
            if (UciOptions[i].type == UCI_OPTION_SPIN) {
                if (intValue < UciOptions[i].min_value) {
                    intValue = UciOptions[i].min_value;
                }
                if (intValue > UciOptions[i].max_value) {
                    intValue = UciOptions[i].max_value;
                }
            }
            
            // Update the option value
            UciOptions[i].current_value = intValue;
            
            // Call the on_change handler if it exists
            if (UciOptions[i].on_change != NULL) {
                UciOptions[i].on_change(intValue);
            }
            
            break;
        }
    }
}

// Print all UCI options
void PrintUciOptions() {
    for (int i = 0; i < UciOptionsCount; i++) {
        switch (UciOptions[i].type) {
            case UCI_OPTION_SPIN:
                printf("option name %s type spin default %d min %d max %d\n",
                       UciOptions[i].name,
                       UciOptions[i].default_value,
                       UciOptions[i].min_value,
                       UciOptions[i].max_value);
                break;
            case UCI_OPTION_BUTTON:
                printf("option name %s type button\n", UciOptions[i].name);
                break;
            case UCI_OPTION_COMBO:
                if (strcmp(UciOptions[i].name, "Style") == 0) {
                    printf("option name %s type combo default %s var %s var %s var %s\n",
                           UciOptions[i].name,
                           StyleNames[STYLE_NORMAL],
                           StyleNames[STYLE_NORMAL],
                           StyleNames[STYLE_AGGRESSIVE],
                           StyleNames[STYLE_SOLID]);
                }
                break;
            // Add cases for other option types
            default:
                break;
        }
    }
}

// Handler for Hash option change
void HashOptionChanged(int value) {
    if (HashTablePtr != NULL) {
        printf("info string set hash to %d MB.\n", value);
        InitHashTable(HashTablePtr, value);
    }
}

// Handler for Clear Hash button press
void ClearHashOptionPressed(int value) {
    if (HashTablePtr != NULL) {
        printf("info string hash table cleared.\n");
        ClearHashTable(HashTablePtr);
    }
}

// Global variable to store thread count
int ThreadCount = DEFAULT_THREADS;

// Handler for Threads option change
void ThreadsOptionChanged(int value) {
    ThreadCount = value;
    printf("info string set threads to %d.\n", value);
}

// Getter for thread count
int GetThreadCount() {
    return ThreadCount;
}

// Handler for Style option change
void StyleOptionChanged(int value) {
    if (value >= 0 && value < STYLE_COUNT) {
        CurrentPlayStyle = (PLAY_STYLE)value;
        printf("info string set style to %s.\n", StyleNames[CurrentPlayStyle]);
    }
}

// Getter for play style
PLAY_STYLE GetPlayStyle() {
    return CurrentPlayStyle;
}

// Get style name as string
const char* GetStyleName(PLAY_STYLE style) {
    if (style >= 0 && style < STYLE_COUNT) {
        return StyleNames[style];
    }
    return "Unknown";
}