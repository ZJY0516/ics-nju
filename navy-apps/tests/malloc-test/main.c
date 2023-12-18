#include <stdio.h>
#include <stdlib.h>

int main()
{
    uint32_t *p = malloc(50 * sizeof(uint32_t));
    printf("addr:%p\n", p);
    printf("addr:%p\n", p + 1);
    printf("addr:%p\n", p + 2);
    printf("addr:%p\n", p + 3);
    printf("addr:%p\n", p + 4);
    printf("addr:%p\n", p + 5);
    printf("addr:%p\n", p + 6);
    printf("addr:%p\n", p + 7);
    return 0;
}