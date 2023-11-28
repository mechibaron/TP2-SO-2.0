#ifdef BUDDY_MM

#include <memoryManager.h>
#define START_ADDRESS 0xF00000
#define MIN_BUDDY_SIZE (8 * 1024)
#define MEMORY_MANAGEMENT_NAME "Buddy"
#define FREE 0
#define OCCUPIED 1

// Estructura para representar un bloque de memoria
typedef struct Block {
    size_t size;
    struct Block *next;
    int is_free;
} Block;

Block memory[MIN_BUDDY_SIZE];

// Inicializa la memoria
void createMemory(size_t size) {
    // Inicializa la memoria con el tamaño proporcionado
    memory[0].size = size;
    memory[0].next = NULL;
    memory[0].is_free = 1;
}

// Divide un bloque en dos partes si es más grande que el tamaño solicitado
void split(Block *block, size_t size) {
    Block *newBlock = (Block *)((char *)block + size);
    newBlock->size = block->size - size;
    newBlock->next = block->next;
    newBlock->is_free = 1;
    block->size = size;
    block->next = newBlock;
}

// Asigna memoria
void *memory_manager_malloc(size_t nbytes) {
    if (nbytes == 0 || nbytes > MIN_BUDDY_SIZE) {
        return NULL;
    }

    size_t allocSize = 1;
    while (allocSize < nbytes) {
        allocSize *= 2;
    }

    Block *block = memory;
    while (block) {
        if (block->is_free && block->size >= allocSize) {
            if (block->size > allocSize) {
                split(block, allocSize);
            }
            block->is_free = 0;
            return (void *)(block + START_ADDRESS + 1); // +1 para saltar la información de bloque
        }
        block = block->next;
    }

    return NULL; // No se pudo asignar memoria
}

// Combina bloques adyacentes libres
void merge() {
    Block *block = memory;
    while (block && block->next) {
        if (block->is_free && block->next->is_free) {
            block->size += block->next->size;
            block->next = block->next->next;
        } else {
            block = block->next;
        }
    }
}

// Libera memoria
void free_memory_manager(void *ap) {
    if (ap == NULL) {
        return;
    }

    Block *block = (Block *)ap - START_ADDRESS - 1;
    if (block->is_free) {
        return;
    }

    block->is_free = 1;
    merge();
}

// Consulta el estado de la memoria
MemoryInfo *mem_info() {
    Block *block = memory;
    int blockCount = 0;
    int freeMemory = 0;
    int usedMemory = 0;

    while (block) {
        blockCount++;
        if (block->is_free) {
            freeMemory += block->size;
        } else {
            usedMemory += block->size;
        }
        block = block->next;
    }

    // Creamos una estructura MemoryInfo con la información del estado de la memoria
    MemoryInfo *info = (MemoryInfo *)malloc(sizeof(MemoryInfo));
    if (info != NULL) {
        info->memoryAlgorithmName = MEMORY_MANAGEMENT_NAME; // Asigna el nombre de tu algoritmo aquí
        info->totalMemory = MIN_BUDDY_SIZE;
        info->occupiedMemory = usedMemory;
        info->freeMemory = freeMemory;
        info->blocksUsed = blockCount;
    }

    return info;
}

#endif