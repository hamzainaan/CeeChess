#include "defs.h"
#include "config.h"
#include "stdio.h"
#include "string.h"

void DebugAnalysisTest(S_BOARD *pos, S_SEARCHINFO *info, S_HASHTABLE *table) {
	FILE *file;
    file = fopen("lct2.epd","r");
    char lineIn [1024];
	info->depth = MAXDEPTH;
	info->timeset = 1;
	int time = 1140000;

    if(file == NULL) {
        printf("File Not Found\n");
        return;
    }  else {
        while(fgets (lineIn , 1024 , file) != NULL) {
			info->starttime = GetTimeMs();
			info->stoptime = info->starttime + time;
			ClearHashTable(table);
            ParseFen(lineIn, pos);
            printf("\n%s\n",lineIn);
			SearchPosition(pos, info, table);
            memset(&lineIn[0], 0, sizeof(lineIn));
        }
    }
}