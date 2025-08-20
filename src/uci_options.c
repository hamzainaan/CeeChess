#include "stdio.h"
#include "string.h"
#include "uci_options.h"
#include "config.h"
#include "defs.h"

// Forward declarations for option handlers
void HashOptionChanged(int value);
void ClearHashOptionPressed(int value);

// UCI options array
S_UCI_OPTION UciOptions[] = {
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
        0,  // default value (not used for buttons)
        0,  // min value (not used for buttons)
        0,  // max value (not used for buttons)
        0,  // current value (not used for buttons)
        ClearHashOptionPressed
    }
    // Add more options here
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
            int intValue = atoi(value);
            
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