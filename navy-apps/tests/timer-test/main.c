#include <stdio.h>
#include <sys/time.h>

int main()
{
    struct timeval tv;
    __time_t sec = 0;
    while (1) {
        gettimeofday(&tv, NULL);
        if (tv.tv_sec > sec) {
            printf("+1s  %d\n", sizeof(tv.tv_sec));
            sec = tv.tv_sec;
        }
    }
    return 0;
}