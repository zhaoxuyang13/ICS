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
#define ADDR_BYTES 8
//#define getSetNo(s) ( s >> blockBits)| (0xf)
#define getCacheLine(s, e)  cacheTable[s * E + e]
 #define IsSpace(c) (c==' '||c=='\t'||c=='\r')
#define skipSpace(p) while(IsSpace(*p)) p++

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
    long addr;
    long  tag;
    long timeStamp;
} CacheLine;
CacheLine*  cacheTable;

typedef struct{
    opType_t opType_t;
    long addr;
    int size;
} LineInfo;
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
    printf("%d,%d,%d, %ld\n", Sets, E , Blocks, sizeof(CacheLine));

    cacheTable = (CacheLine *)calloc(Sets * E , sizeof(CacheLine));
    for(int i = 0; i < Sets * E ; i ++)
        cacheTable[i].isValid = FALSE;   //cold cache
    currentTime = 0;
    printf("init complete index of cache table : %ld\n",sizeof(cacheTable[Sets*E -1]) );
 }


 long parseAddr(char * line, int lenth)
 {
        long ret = 0;
        while(lenth > 0){
            if(*line <= '9' && *line >='0')
                ret += *line - '0';
            else{
                switch(*line)
                {
                    case 'a': ret += 10; break;
                    case 'b': ret += 11; break;
                    case 'c': ret += 12 ; break;
                    case 'd': ret += 13; break;
                    case 'e': ret += 14; break;
                    case 'f' : ret += 15; break;
                    default: break;
                }
            }
            lenth --;
            line ++;
            if(lenth >= 1) ret <<= 4;
        }
        return ret;
 }
 LineInfo parseLine(char * line)
 {
        LineInfo info;
        /* type*/
        if(IsSpace(line[0]))
        {
            skipSpace(line);
            switch(line[1])
            {
                case 'M':  info.opType_t = Mod; break;
                case 'L' : info.opType_t = Load; break ;
                case 'S' : info.opType_t =Store; break ;
            }
            line ++;
        }
        else
        {
            info.opType_t = Inst;
            line ++;
        }
        /*addr*/
        skipSpace(line);
        info.addr = strtoul(line,&line, 16);
        /*size*/
        line ++; // skip ',';
        info.size= strtoul(line, &line, 10);
        return info;
 }
void processLine(char *line){
        LineInfo info  = parseLine(line);
        printf("%d, %ld, %d \n", info.opType_t, info.addr,info.size);




}
void processFile(){
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
            processLine(line);
        }
}
int main(int argc, char *argv[])
{
     parseMainArg(argc, argv);
     initCache();
     processFile();

    return 0;
}
