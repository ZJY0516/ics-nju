#include <stdio.h>
#include <sys/time.h>
#include <NDL.h>

#define NDL

int main()
{
#ifdef NDL
    uint32_t msec = 0;
#elif
    struct timeval tv;
    __time_t sec = 0;
#endif
    while (1) {
#ifdef NDL
        if (NDL_GetTicks() > msec + 500) {
            printf("msec: %u\n", NDL_GetTicks());
            msec = NDL_GetTicks();
        }
#elif
        gettimeofday(&tv, NULL);
        if (tv.tv_sec > sec) {
            printf("+1s  %d\n", sizeof(tv.tv_sec));
            printf("gettimeofday: %ld.%06ld\n", tv.tv_sec, tv.tv_usec);
            sec = tv.tv_sec;
        }
#endif
    }
    return 0;
}