#include <pipe.h>

pipeList pipesList = NULL;

//funcion para abrir un pipe nuevo
Pipe *pipeOpen()
{
    //incluimos en el newpipe las variables de pipe e inicializamos
    Pipe *newPipe = (Pipe *)memory_manager_malloc(sizeof(Pipe));
    newPipe->isReadOpen = 1;
    newPipe->isWriteOpen = 1;
    newPipe->readQueue = newQueue();
    newPipe->writeQueue = newQueue();
    newPipe->processCount = 1;

    pipeNode *newPipeNode = (pipeNode *)memory_manager_malloc(sizeof(pipeNode));
    newPipeNode->pipe = newPipe;
    newPipeNode->next = pipesList;
    newPipeNode->previous = NULL;
    pipesList = newPipeNode;
    return newPipe;
}


int pipeClose(Pipe *pipe) {
    if (pipe->processCount > 1)
    {
        pipe->processCount--;
    }
    pipeNode *current = pipesList;
    while (current != NULL && current->pipe != pipe)
    {
        current = current->next;
    }
    if (current == NULL)
    {
        return 0;
    }

    freeQueue(pipe->readQueue);
    freeQueue(pipe->writeQueue);
    if (current->next != NULL)
    {
        current->next->previous = current->previous;
    }
    if (current->previous != NULL)
    {
        current->previous->next = current->next;
    }
    free_memory_manager(current);
    free_memory_manager(pipe);
    return 1;
}

//devuelve la cantidad de chars leidos y sino -1
int pipeReadData(Pipe *pipe, char *msg, int size) {
    if (!pipe->isReadOpen) {
        return -1;
    }

    int i;
    pid_t currentPid;
    for (i = 0; i < size; i++) {
        //verificamos que el readIndex sea igual al writeIndex
        if (pipe->readIndex == pipe->writeIndex) {
            //si lo son, es porque no hay datos para leer en el pipe
            currentPid = getCurrentPid();
            //se agrega a la readQueue hasta que haya datos disponibles para leer
            enqueuePid(pipe->readQueue, currentPid);
            //el proceso actual se blquea
            blockProcess(currentPid);
        }
        //si hay datos, se lee un char y se almacena en msg
        msg[i] = pipe->data[pipe->readIndex % PIPESIZE];
        pipe->readIndex++;
        //despues de cada lectura verificamos si hay procesos bloqueados esperando
        //para escribir en el tubo
        while ((currentPid = dequeuePid(pipe->writeQueue)) != -1) {
            //si hay, se desbloquean y se sacan de la writeQueue
            unblockProcess(currentPid);
        }
    }

    return i;
}


//devuelve la cantidad de chars escritos y sino -1
//funciona de manera similar que el pipeReadData
int pipeWriteData(Pipe *pipe, const char *msg, int size) {
    if (!pipe->isWriteOpen) {
        return -1;
    }

    int i;
    pid_t currentPid;

    for (i = 0; i < size; i++) {
        if (pipe->writeIndex == pipe->readIndex + PIPESIZE) {
            currentPid = getCurrentPid();
            enqueuePid(pipe->writeQueue, currentPid);
            blockProcess(currentPid);
        }

        pipe->data[pipe->writeIndex % PIPESIZE] = msg[i];
        pipe->writeIndex++;

        while ((currentPid = dequeuePid(pipe->readQueue)) != -1) {
            unblockProcess(currentPid);
        }
    }

    return 1;
}