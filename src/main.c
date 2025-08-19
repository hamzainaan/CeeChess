// main.c

#include "stdio.h"
#include "defs.h"
#include "stdlib.h"
#include "string.h"
#include "uci_options.h"
#include "config.h"

int main() {

	AllInit();
	S_BOARD pos[1];
    S_SEARCHINFO info[1];
    info->quit = 0;
	HashTable->pTable = NULL;
    InitHashTable(HashTable, DEFAULT_HASH_SIZE);
	setbuf(stdin, NULL);
    setbuf(stdout, NULL);

	char line[256];
	while (1) {
		memset(&line[0], 0, sizeof(line));
		fflush(stdout);
		if (!fgets(line, 256, stdin))
			continue;
		if (line[0] == '\n')
			continue;
		if (!strncmp(line, "uci",3)) {
			Uci_Loop(pos, info, HashTable);
			if(info->quit == 1) break;
			continue;
		} else if(!strncmp(line, "quit",4))	{
			break;
		}
	}

	free(HashTable->pTable);
	return 0;
}
