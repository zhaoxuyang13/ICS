/*
*   Student Name: Zhao Xuyang
*   Student ID: 517021911099
*
*/
#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#define error(s) printf("Error:%s \n", s)
#define MAX_LINE_LENTH 100
//#define getSetNo(s) ( s >> blockBits)| (0xf)
#define getCacheLine(s, e)  cacheTable[s * E + e]
typedef enum {FALSE, TRUE } bool_t;
typedef enum {Inst, Load, Store, Mod} opType_t;

/*
 main config  -s -E -b -t
*/
int setBits, E, blockBits, Sets,Blocks;
char *fileLocation;
bool_t verbose = FALSE;
/*
cache table
*/
int currentTime; // identify timeStamp.  increment at every new op
typedef struct{
     bool_t isValid;
    char *addr;
    int tag;
    int timeStamp;
} cache_line;
cache_line*  cacheTable;

extern int optind,opterr,optopt;
extern char* optarg;
extern int getopt(int argc, char *const*argv, const char * optstring);

void parseMainArg(int argc, char *argv[]){
    char ch;
    while((ch = getopt(argc,argv,"hvs:E:b:t:"))  != -1)
    {
        switch(ch)
        {
            case 'h':
                printf("Some usage info \n");
                break;
            case 'v':
                verbose = TRUE;
                break;
            case 's':
                setBits = atoi(optarg);
                Sets = 1 << setBits;
                break;
            case'E':
                E = atoi(optarg);
                break;
            case 'b':
                blockBits = atoi(optarg);
                Blocks = 1 << blockBits;
                break;
            case 't':
                fileLocation = optarg;
                break;
            default:
                printf("unknown option: %c \n", (char)optopt);
        }
    }
}


void initCache(){
    cacheTable = malloc(Sets * E * sizeof(cache_line));
    for(int i = 0; i < Sets * E ; i ++)
        cacheTable[i].isValid = FALSE;   //cold cache
    currentTime = 0;
    int indexNum = sizeof(cacheTable)/ sizeof(cache_line);
    printf("init complete index of cache table : %u\n", indexNum);
 }

void parseLine(char *line){

}
void parseFile(){
        FILE *fp  = NULL;
        if((fp = fopen(fileLocation, "r")) == NULL)
        {
            printf("can't find file.");
            exit(1);
        }
        char line[MAX_LINE_LENTH];
        while(!feof(fp))
        {
            fgets(line,MAX_LINE_LENTH,fp);
            parseLine(line);
        }
}
int main(int argc, char *argv[])
{
     parseMainArg(argc, argv);
     initCache();
     parseFile();

    return 0;
}
