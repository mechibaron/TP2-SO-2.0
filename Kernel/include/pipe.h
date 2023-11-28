#include <defs.h>
#include <memoryManager.h>
#include <queue.h>
#include <scheduler.h>

Pipe *pipeOpen();
int pipeClose(Pipe *pipe);
int pipeReadData(Pipe *pipe, char *msg, int size);
int pipeWriteData(Pipe *pipe, const char *msg, int size);
