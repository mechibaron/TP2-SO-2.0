#include <scheduler.h>
#include <interrupts.h>
#include <memoryManager.h>
#include <queue.h>
#include <lib.h>
#include <pipe.h>


extern uint64_t loadProcess(uint64_t rip, uint64_t rsp, uint64_t argc, uint64_t argv); // implement on assembler
extern void _int20h();                                                                 // implement int20h con assembler

// tck and ppriorities
#define STACK_SIZE 4096
#define MIN_PRIORITY 1
#define MAX_PRIORITY 9
#define EOF -1

#define NUMBER_OF_PRIORITIES 9
#define DEFAULT_PRIORITY 4
priority_t priorities[NUMBER_OF_PRIORITIES] = {9, 8, 7, 6, 5, 4, 3, 2, 1};

// Queues
Queue active = NULL;
Queue expired = NULL;

// Schelduler states
int processAmount = -1;
unsigned int processReadyCount = 0;
pid_t dummyProcessPid = 1;
char proccessBeingRun = 0;
int readyProcessAmount = 0;

typedef struct {
    char *name;
    int fileDescriptors[5];
    int lastFd;
    int status;
} Process;

typedef void (*function_ptr)();

void dummyProcess() {
    while (1) {
        _hlt();
    }
}

void createScheduler()
{
    dummyProcessPid = new_process((uint64_t)dummyProcess, 0, NULL);
    for (int i = 0; i <= 2; i++)
    {
        active->process.fileDescriptors[i].mode = OPEN;
    }
    // PIPEOUT, PIPEIN
    for (int i = 3; i <= 4; i++)
    {
        active->process.fileDescriptors[i].mode = CLOSED;
    }
    active->process.lastFd = 4;
    active->process.status = BLOCKED;
    processReadyCount--;
}

PCB *getProcess(pid_t pid) {
    Node *current = active;

    while (current != NULL) {
        if (current->process.pid == pid) {
            return &(current->process);
        }
        current = current->next;
    }

    current = expired;

    while (current != NULL) {
        if (current->process.pid == pid) {
            return &(current->process);
        }
        current = current->next;
    }

    return NULL;
}

uint64_t getCurrentPid()
{
    if (active != NULL)
    {
        return active->process.pid;
    }
    return -1;
}

int findAndSetStatus(Node *current, pid_t pid, int status) {
    while (current != NULL) {
        if (current->process.pid == pid) {
            current->process.status = status;
            return 1;
        }
        current = current->next;
    }
    return 0;
}

int blockProcess(pid_t pid) {
    Node *current;

    current = active;
    if (findAndSetStatus(current, pid, BLOCKED)) {
        processReadyCount--;
        _int20h();
        return 0;
    }

    current = expired;
    if (findAndSetStatus(current, pid, BLOCKED)) {
        processReadyCount--;
        _int20h();
        return 0;
    }

    return -1;
}

int unblockProcess(pid_t pid) {
    Node *current;
    int statusUpdated = 0;

    // Busca y actualiza el estado en la lista 'active'
    current = active;
    statusUpdated = findAndSetStatus(current, pid, READY);

    // Si el estado ya se actualizó, no es necesario buscar en la lista 'expired'
    if (statusUpdated) {
        processReadyCount++;
        return 0;
    }

    // Si no se encontró en 'active', busca y actualiza el estado en la lista 'expired'
    current = expired;
    statusUpdated = findAndSetStatus(current, pid, READY);

    if (statusUpdated) {
        processReadyCount++;
        return 0;
    }

    return -1;
}

char **copy_argv(int argc, char **argv) {

    char **new_argv = memory_manager_malloc(sizeof(char *) * argc);

    for (int i = 0; i < argc; i++) {
        int length = strlen(argv[i]);
        //asigna memoria para la nueva cadena
        new_argv[i] = memory_manager_malloc(length + 1); 
        //copiamos la cadena original a la nueva ubicacion
        new_argv[i] = strcpy(argv[i]); 
    }

    return new_argv;
}


pid_t new_process(uint64_t rip, int argc, char *argv[]) {
    Node *newProcess = memory_manager_malloc(sizeof(Node));
    newProcess->process.pid = processAmount++;
    newProcess->process.priority = DEFAULT_PRIORITY;
    newProcess->process.quantumsLeft = priorities[DEFAULT_PRIORITY];
    newProcess->process.blockedQueue = newQueue();
    newProcess->process.newPriority = -1;
    newProcess->process.status = READY;
    newProcess->process.argc = argc;
    newProcess->process.argv = copy_argv(argc, argv);

     if (active != NULL)
    {
        for (int i = 0; i <= active->process.lastFd; i++)
        {
            newProcess->process.fileDescriptors[i].mode = active->process.fileDescriptors[i].mode;
        }
        newProcess->process.lastFd = active->process.lastFd;
        newProcess->process.pipe = active->process.pipe;
    }

    uint64_t rsp = (uint64_t)memory_manager_malloc(4 * 1024);
    if (rsp == 0) {
        return -1;
    }
    newProcess->process.stackBase = rsp;
    uint64_t newRsp = (uint64_t)loadProcess(rip, rsp + 4 * 1024, newProcess->process.argc, (uint64_t)newProcess->process.argv);
    newProcess->process.rsp = newRsp;

    Node *newProcessNode = memory_manager_malloc(sizeof(Node));
    newProcessNode->process = newProcess->process;
    newProcessNode->next = NULL;

    if (active == NULL) {
        active = newProcessNode;
    } else {
        Node *current = expired;
        Node *previous = NULL;

        while (current != NULL && newProcess->process.priority >= current->process.priority) {
            previous = current;
            current = current->next;
        }

        if (previous == NULL) {
            newProcessNode->next = expired;
            expired = newProcessNode;
        } else {
            newProcessNode->next = current;
            previous->next = newProcessNode;
        }
    }

    processReadyCount++;
    return newProcess->process.pid;
}

void nextProcess() {
    Node *current = active;
    Node *previous = NULL;

    // Busca un proceso no bloqueado en la lista activa
    while (current != NULL && current->process.status == BLOCKED) {
        previous = current;
        current = current->next;
    }

    // Si se encuentra un proceso no bloqueado en la lista activa
    if (current != NULL) {
        if (previous != NULL) {
            // Elimina el proceso de su posición actual
            previous->next = current->next;
            // Coloca el proceso al inicio de la lista activa
            current->next = active;
            active = current;
        }
    } else {
        // Intercambia las listas activa y caducada
        Node *aux = active;
        active = expired;
        expired = aux;

        current = active;
        previous = NULL;

        // Busca un proceso no bloqueado en la lista activa
        while (current != NULL && current->process.status == BLOCKED) {
            previous = current;
            current = current->next;
        }

        // Si se encuentra un proceso no bloqueado en la lista activa
        if (previous != NULL && current != NULL) {
            // Elimina el proceso de su posición actual
            previous->next = current->next;
            // Coloca el proceso al inicio de la lista activa
            current->next = active;
            active = current;
        }
    }
}



Node *findProcessInList(Node *list, pid_t pid) {
    Node *current = list;
    // Node *previous = NULL;
    while (current != NULL && current->process.pid != pid) {
        // previous = current;
        current = current->next;
    }
    return current;
}

void moveProcessToActive(Node *current, Node *previous) {
    if (previous != NULL) {
        previous->next = current->next;
        current->next = active;
        active = current;
    }
}

void moveProcessToActiveFromExpired(Node *current, Node *previous) {
    if (previous == NULL) {
        expired = current->next;
    } else {
        previous->next = current->next;
    }
    current->next = active;
    active = current;
}

int prepareDummy(pid_t pid) {
    Node *current;
    Node *previous = NULL;

    // Busca el proceso en la lista activa
    current = findProcessInList(active, pid);

    // Si se encuentra en la lista activa
    if (current != NULL) {
        moveProcessToActive(current, previous);
    } else {
        // Busca el proceso en la lista caducada
        current = findProcessInList(expired, pid);

        // Si no se encuentra en la lista caducada
        if (current == NULL) {
            return -1;
        }

        moveProcessToActiveFromExpired(current, previous);
    }

    return 0;
}

void moveProcessToExpired(Node *currentProcess) {
    Node *currentExpired = expired;
    Node *previousExpired = NULL;

    while (currentExpired != NULL && currentProcess->process.priority >= currentExpired->process.priority) {
        previousExpired = currentExpired;
        currentExpired = currentExpired->next;
    }

    active = active->next;

    if (previousExpired == NULL) {
        currentProcess->next = expired;
        expired = currentProcess;
    } else {
        previousExpired->next = currentProcess;
        currentProcess->next = currentExpired;
    }

    if (active == NULL) {
        active = expired;
        expired = NULL;
    }
}

uint64_t contextSwitch(uint64_t rsp) {
    if (!proccessBeingRun) {
        proccessBeingRun = 1;
        if (processReadyCount > 0) {
            nextProcess();
        } else {
            prepareDummy(dummyProcessPid);
        }
        return active->process.rsp;
    }

    Node *currentProcess = active;
    currentProcess->process.rsp = rsp;

    if (processReadyCount == 0) {
        prepareDummy(dummyProcessPid);
        return active->process.rsp;
    }

    if (currentProcess->process.status != BLOCKED && currentProcess->process.quantumsLeft > 0) {
        currentProcess->process.quantumsLeft--;
        return rsp;
    }

    if (currentProcess->process.quantumsLeft == 0) {
        if (currentProcess->process.newPriority != -1) {
            currentProcess->process.priority = currentProcess->process.newPriority;
            currentProcess->process.newPriority = -1;
        }
        currentProcess->process.quantumsLeft = priorities[currentProcess->process.priority];

        moveProcessToExpired(currentProcess);
        nextProcess();
        return active->process.rsp;
    }
    //por si ninguna condicion se cumple
    return -1;
}




int killProcess(int returnValue, char autokill) {
    Node *currentProcess = active;

    // Desbloquear procesos bloqueados
    pid_t blockedPid;
    while ((blockedPid = dequeuePid(currentProcess->process.blockedQueue)) != -1)
    {
        unblockProcess(blockedPid);
    }

    // Actualizar la lista activa
    active = currentProcess->next;

    // Reducir el contador de procesos listos si no estaba bloqueado
    if (currentProcess->process.status != BLOCKED) {
        processReadyCount--;
    }

    // Liberar la memoria utilizada por los argumentos
    for (int i = 0; i < currentProcess->process.argc; i++) {
        free_memory_manager(currentProcess->process.argv[i]);
    }
    free_memory_manager(currentProcess->process.argv);

    // Liberar la cola bloqueada y la memoria de la pila
    freeQueue(currentProcess->process.blockedQueue);
    free_memory_manager((void *)currentProcess->process.stackBase);

    // Si existe una tubería, escribir un mensaje EOF
    if (currentProcess->process.pipe != NULL) {
        char msg[1] = {EOF};
        pipeWriteData(currentProcess->process.pipe, msg, 1);
    }

    // Liberar la memoria utilizada por el proceso
    free_memory_manager(currentProcess);

    if (autokill) {
        proccessBeingRun = 0;
        _int20h();
    }

    return returnValue;
}

void setProcessNewPriority(PCB *process, int priorityValue) {
    process->newPriority = priorityValue;
}

void resetProcessQuantumsLeft(PCB *process) {
    process->quantumsLeft = 0;
}
int changePriority(pid_t pid, int priorityValue) {
    if (priorityValue < 0 || priorityValue > 8) {
        return -1;
    }

    PCB *process = getProcess(pid);

    if (process != NULL) {
        setProcessNewPriority(process, priorityValue);
        return 0;
    }

    return -1;
}

int yieldProcess() {
    resetProcessQuantumsLeft(&active->process);
    _int20h();
    return 0;
}

processInfo *addProcessInfoNode(processInfo *current, PCB process) {
    processInfo *newNode = (processInfo *)memory_manager_malloc(sizeof(processInfo));
    newNode->pid = process.pid;
    newNode->priority = process.priority;
    newNode->stackBase = process.stackBase;
    newNode->status = process.status;

    if (current != NULL) {
        current->next = newNode;
    }

    return newNode;
}

processInfo *getProcessesInfo() {
    processInfo *first = NULL;
    processInfo *current = NULL;

    // Obtener información de la lista activa
    Queue currentNode = active;
    while (currentNode != NULL) {
        current = addProcessInfoNode(current, currentNode->process);
        if (first == NULL) {
            first = current;
        }
        currentNode = currentNode->next;
    }

    // Obtener información de la lista caducada
    currentNode = expired;
    while (currentNode != NULL) {
        current = addProcessInfoNode(current, currentNode->process);
        if (first == NULL) {
            first = current;
        }
        currentNode = currentNode->next;
    }

    if (current != NULL) {
        current->next = NULL;
    }

    return first;
}


processInfo *getProccessesInfo()
{
    processInfo *first = NULL;
    processInfo *current = NULL;
    Queue currentNode = active;
    pid_t firstPid = active->process.pid;

    while (currentNode != NULL)
    {
        if (current != NULL)
        {
            current->next = (processInfo *)memory_manager_malloc(sizeof(processInfo));
            current = current->next;
        }
        else
        {
            current = (processInfo *)memory_manager_malloc(sizeof(processInfo));
        }
        current->pid = currentNode->process.pid;
        if (current->pid == firstPid)
        {
            first = current;
        }
        current->priority = currentNode->process.priority;
        current->stackBase = currentNode->process.stackBase;
        current->status = currentNode->process.status;
        currentNode = currentNode->next;
    }
    currentNode = expired;

    while (currentNode != NULL)
    {
        if (current != NULL)
        {
            current->next = (processInfo *)memory_manager_malloc(sizeof(processInfo));
            current = current->next;
        }
        else
        {
            current = (processInfo *)memory_manager_malloc(sizeof(processInfo));
        }
        current->pid = currentNode->process.pid;
        current->priority = currentNode->process.priority;
        current->stackBase = currentNode->process.stackBase;
        current->status = currentNode->process.status;
        currentNode = currentNode->next;
    }
    current->next = NULL;
    return first;
}
