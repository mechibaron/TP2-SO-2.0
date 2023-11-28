#include <semaphore.h>
#include <memoryManager.h>
#include <queue.h>
#include <scheduler.h>
extern int block(uint8_t *lock); // Both of these functions will be used to avoid
extern void unblock(uint8_t *lock);  // race conditions in the post and wait

typedef struct semNode
{
    semaphore sem;
    struct semNode *next;
    struct semNode *previous;
} semNode;

typedef semNode *semList;

semList semaphoreList = NULL;

semNode *findSemaphore(char *name)
{
    semNode *current = semaphoreList;
    while (current != NULL)
    {
        if (strcmp(current->sem.name, name) == 0)
        {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// Creates semaphore
// If same with name already exists already exist, doesnt change value
sem_t sem_open(char *name, uint64_t value)
{
    semNode *semAux = findSemaphore(name);
    if (semAux != NULL)
    {
        semAux->sem.processesOpened++;
        return &(semAux->sem);
    }
    semAux = (semNode *)memory_manager_malloc(sizeof(semNode));
    semAux->sem.name = strcpy(name);
    semAux->sem.value = value;
    semAux->sem.locked = 0;
    semAux->sem.processesOpened = 1;
    semAux->next = semaphoreList;
    semAux->previous = NULL;
    semAux->sem.blockedProcesses = newQueue();
    semaphoreList = semAux;
    return &(semAux->sem);
}

// Removes a semaphore
// 1 if it could remove it, 0 if it wasnt removed but didnt fail (just removed it in this process) -1 if it failed
int sem_close(sem_t sem)
{
    semNode *semAux = findSemaphore(sem->name);
    if (semAux == NULL)
    {
        return -1;
    }

    if (sem->processesOpened > 1)
    {
        sem->processesOpened--;
        return 0;
    }

    if (semAux->previous != NULL)
    {
        semAux->previous->next = semAux->next;
    }
    if (semAux->next != NULL)
    {
        semAux->next->previous = semAux->previous;
    }

    freeQueue(sem->blockedProcesses);
    free_memory_manager(sem->name);
    free_memory_manager(semAux);
    return 1;
}

// Adds 1 to the value of a semaphore, if it was blocked (==0) it unblocks one of the blocked process
// Returns 1 on success, -1 on error
int sem_post(sem_t sem)
{
    semNode *semAux = findSemaphore(sem->name);

    if (semAux == NULL)
    {
        return -1;
    }

    block(&(sem->locked));
    if (sem->blockedProcesses->size != 0)
    {
        pid_t pidCurrent = dequeuePid(sem->blockedProcesses);
        unblockProcess(pidCurrent);
    }
    else
    {
        sem->value++;
    }
    unblock(&(sem->locked));

    return 1;
}

// Reduce the value of the semaphore by 1, if it is now 0 it is blocked
// Returns 1 on success, -1 on error
int sem_wait(sem_t sem)
{
    semNode *semAux = findSemaphore(sem->name);

    if (semAux == NULL)
    {
        return -1;
    }

    block(&(sem->locked));
    if (sem->value > 0)
    {
        sem->value--;
        unblock(&(sem->locked));
    }
    else
    {
        pid_t pidCurrent = getCurrentPid();
        enqueuePid(sem->blockedProcesses, pidCurrent);
        unblock(&(sem->locked));
        blockProcess(pidCurrent);
    }
    return 1;
}