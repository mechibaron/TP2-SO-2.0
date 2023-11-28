
#ifndef _defs_
#define _defs_

#include <stdint.h>
#include <stddef.h>


#define NULL ((void *)0)

/* Flags para derechos de acceso de los segmentos */
#define ACS_PRESENT     0x80            /* segmento presente en memoria */
#define ACS_CSEG        0x18            /* segmento de codigo */
#define ACS_DSEG        0x10            /* segmento de datos */
#define ACS_READ        0x02            /* segmento de lectura */
#define ACS_WRITE       0x02            /* segmento de escritura */
#define ACS_IDT         ACS_DSEG
#define ACS_INT_386 	  0x0E		        /* Interrupt GATE 32 bits */
#define ACS_INT         ( ACS_PRESENT | ACS_INT_386 )


#define ACS_CODE        (ACS_PRESENT | ACS_CSEG | ACS_READ)
#define ACS_DATA        (ACS_PRESENT | ACS_DSEG | ACS_WRITE)
#define ACS_STACK       (ACS_PRESENT | ACS_DSEG | ACS_WRITE)

/* Registros en memoria en las syscalls */
#define REG_AMOUNT 15
#define RIP_POS REG_AMOUNT
#define RFLAGS_POS (REG_AMOUNT + 2)
#define RSP_POS (REG_AMOUNT + 3)

#define PIPESIZE 512
#define FDS 10
#define OPEN 1
#define CLOSED 0

// #define EOF -1

typedef int pid_t;
typedef struct blockednode {
    pid_t pid;
    struct blockednode *next;
} blockednode;

typedef struct {
    blockednode *head;
    blockednode *tail;
    unsigned int size;
} BlockedQueueCDT;


typedef BlockedQueueCDT *BlockedQueueADT;

typedef struct
{
    char *name;
    uint64_t value; // it wont be negative, process that try to wait when 0 will be stacked in blockedProcess
    uint64_t processesOpened;
    uint8_t locked; // if its locked its 1 if not 0;
    BlockedQueueADT blockedProcesses;
} semaphore;

typedef semaphore *sem_t;

typedef struct Pipe
{
    char data[PIPESIZE];
    unsigned int processCount;
    unsigned int isReadOpen;
    unsigned int isWriteOpen;
    uint64_t readIndex;
    uint64_t writeIndex;
    BlockedQueueADT writeQueue;
    BlockedQueueADT readQueue;
} Pipe;

typedef struct pipeNode
{
    Pipe *pipe;
    struct pipeNode *next;
    struct pipeNode *previous;
} pipeNode;

typedef struct PipeList {
    pipeNode *head;
    pipeNode *tail;
} PipeList;

typedef pipeNode *pipeList;

typedef struct
{
    unsigned int mode;
} fd_t;


typedef unsigned int priority_t;
typedef unsigned int status_t;

typedef struct
{
    pid_t pid;
    priority_t priority;
    int newPriority;
    status_t status;
    unsigned int quantumsLeft;
    uint64_t rsp;
    uint64_t stackBase;
    BlockedQueueADT blockedQueue;
    fd_t fileDescriptors[FDS];
    Pipe *pipe;
    unsigned int lastFd;
    unsigned int argc;
    char **argv;
} PCB;

typedef struct node
{
    PCB process;
    struct node *next;
} Node;

typedef Node *Queue;

typedef struct processInfo
{
    pid_t pid;
    priority_t priority;
    uint64_t stackBase;
    status_t status;
    struct processInfo *next;
} processInfo;



typedef struct memoryInfo
{
    char *memoryAlgorithmName;
    size_t freeMemory;
    size_t occupiedMemory;
    size_t totalMemory;
    unsigned int blocksUsed;
} MemoryInfo;

// typedef long Align;
// typedef union header {
//     struct {
//         union header* ptr;
//         uint64_t size;
//     } data;

//     Align x;
// } header;

#endif