#ifndef memoryManagerFreeH
#define memoryManagerFreeH

#include <unistd.h>
#include <stdint.h>
#include <defs.h>


void createMemory(size_t size);
void *memory_manager_malloc(size_t nbytes);
void free_memory_manager(void *ap);
MemoryInfo *mem_info();
#endif