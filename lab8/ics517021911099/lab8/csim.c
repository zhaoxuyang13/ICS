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
long currentTime; // identify timeStamp.  increment at every new op
typedef struct{
     bool_t isValid;
    long  tag;
    long timeStamp;
} CacheLine;
CacheLine*  cacheTable;

typedef struct{
    opType_t opType;
    long addr;
    int size;
} LineInfo;

/*
counters;
*/
int hits, misses, evicts;


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
    hits= 0;
    misses= 0;
    evicts = 0;
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
        if(IsSpace(*line))
        {
            skipSpace(line);
            switch(*line)
            {
                case 'M':
                    info.opType = Mod; break;
                case 'L' :
                    info.opType = Load; break;
                case 'S' :
                    info.opType =Store; break;
            }
            line ++;
        }
        else
        {
            info.opType= Inst;
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
 /*
 if         hit     return index
            miss  return NULL

 */
int findCache(int setNo, long tag)
{
     for(int i = setNo * E; i < (setNo+1)*E; i ++)
    {
        if(cacheTable[i].isValid == FALSE)
            continue;
        if(tag == cacheTable[i].tag)
            return i;
    }
    return FALSE;
}
char OPTYPE[4] = {'I', 'L','S','M'};
void doCache(LineInfo info)
{
        int setNo = ( info.addr >> blockBits ) &  (0xffffffff >>( 32-setBits));
        //printf("setNo: %x ", setNo);
        long tag = info.addr >> (blockBits + setBits);
        //printf("tag: %ld\n", tag);
        if(info.opType == Load || info.opType == Store || info.opType == Mod)
        {
            int index =findCache(setNo,tag);
            if(index)
            {
                hits++;
                cacheTable[index].timeStamp = ++currentTime;
                printf("%c %lx,%d hit",OPTYPE[info.opType], info.addr, info.size);
            }
            else
            {
                misses++;
                bool_t haveColdCache = FALSE;
                for(int i = setNo * E; i < (setNo+1)*E; i ++)
                {
                    if(!cacheTable[i].isValid) // find coldcache
                    {
                        haveColdCache = TRUE;
                        CacheLine tmp;
                        tmp.isValid = TRUE;
                        tmp.tag = tag;
                        tmp.timeStamp = ++currentTime;
                        cacheTable[i] = tmp;
                    }
                }
                if(!haveColdCache)  // start eviction
                {
                    evicts ++ ;
                    long LRTime = currentTime + 1;
                    int LRIndex = -1;
                    for(int i = setNo * E; i < (setNo+1)*E; i ++)
                    {
                        if(cacheTable[i].timeStamp < LRTime)
                        {
                            LRTime = cacheTable[i].timeStamp;
                            LRIndex = i;
                        }
                    }
                    CacheLine tmp;
                    tmp.isValid = TRUE;
                    tmp.tag = tag;
                    tmp.timeStamp = ++currentTime;
                    cacheTable[LRIndex] = tmp;
                    printf("%c %lx,%d miss eviction",OPTYPE[info.opType], info.addr, info.size);
                }
                else printf("%c %lx,%d miss",OPTYPE[info.opType], info.addr, info.size);
            }
        }
        if(info.opType == Mod)
        {
            hits++;
            printf(" hit");
        }
        printf("\n");
}
void processLine(char *line){
        LineInfo info  = parseLine(line);
        //printf("%d, %lx, %d \n", info.opType, info.addr,info.size);
        doCache(info);
}
void processFile(){
        FILE *fp  = NULL;
        if((fp = fopen(fileLocation, "r")) == NULL)
        {
            error("cannot_find_file");
            exit(1);
        }
        char line[MAX_LINE_LENTH];
        while(fgets(line,MAX_LINE_LENTH,fp))
        {
            processLine(line);
        }
}
int main(int argc, char *argv[])
{
     parseMainArg(argc, argv);
     initCache();
     processFile();
    printSummary(hits,misses,evicts);
    return 0;
}
