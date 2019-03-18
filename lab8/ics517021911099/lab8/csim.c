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


typedef enum {FALSE, TRUE } bool_t;
typedef enum {Inst, Load, Store, Mod} opType_t;

int setBits, E, blockBits, Sets,Blocks;
char *fileLocation;
bool_t verbose = FALSE;

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

}
void parseFile(){

}
int main(int argc, char *argv[])
{
     parseMainArg(argc, argv);
     initCache();
     parseFile();
    return 0;
}
