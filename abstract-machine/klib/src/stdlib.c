#include <am.h>
#include <klib.h>
#include <klib-macros.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
static unsigned long int next = 1;

int rand(void)
{
    // RAND_MAX assumed to be 32767
    next = next * 1103515245 + 12345;
    return (unsigned int)(next / 65536) % 32768;
}

void srand(unsigned int seed) { next = seed; }

int abs(int x) { return (x < 0 ? -x : x); }

int atoi(const char *nptr)
{
    int x = 0;
    while (*nptr == ' ') {
        nptr++;
    }
    while (*nptr >= '0' && *nptr <= '9') {
        x = x * 10 + *nptr - '0';
        nptr++;
    }
    return x;
}
static char *addr = NULL;
// #define HEAP_SIZE 1000000
// static uint8_t mem[HEAP_SIZE] = {0};
// typedef struct heap {
//     uint8_t *start;
//     uint8_t *end;
//     uint8_t *ptr;
//     uint8_t *mem;
//     uint32_t size;
//     /* data */
// } Heap;
// static Heap mem_heap = {
//     .mem = mem,
//     .start = mem,
//     .end = mem + HEAP_SIZE - 1,
//     .ptr = mem,
//     .size = HEAP_SIZE,
// };
void *malloc(size_t size)
{
    // On native, malloc() will be called during initializaion of C runtime.
    // Therefore do not call panic() here, else it will yield a dead recursion:
    //   panic() -> putchar() -> (glibc) -> malloc() -> panic()
#if !(defined(__ISA_NATIVE__) && defined(__NATIVE_USE_KLIB__))
    // panic("Not implemented");
    if (addr == NULL)
        addr = heap.start;
    size_t alignment = sizeof(size_t); // memory alignment
    size_t aligned_size = (size + alignment - 1) / alignment * alignment;
    char *ret = addr;
    addr += aligned_size;
    // printf("malloc: %u\n", ret);
    return ret;
    // uint8_t *old = mem_heap.ptr;
    // mem_heap.ptr += size;
    // assert(mem_heap.ptr <= mem_heap.end);
    // return (void *)old;
#endif
    return NULL;
}

void free(void *ptr) {}

#endif
