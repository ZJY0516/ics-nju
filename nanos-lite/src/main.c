#include <common.h>

void init_mm(void);
void init_device(void);
void init_ramdisk(void);
void init_irq(void);
void init_fs(void);
void init_proc(void);

int main()
{
    extern const char logo[];
    printf("%s", logo);
    printf("111111111\n");
    // Log("'Hello World!' from Nanos-lite");
    //  Log("Build time: %s, %s", __TIME__, __DATE__);
    printf("\33[1;35m[%s,%d,%s] "
           "Build time: %s, %s"
           "\33[0m\n",
           "/home/zjy/code/ics2023/nanos-lite/src/main.c", 16, __func__,
           "16:06:02", "Dec 20 2023");
    init_mm();

    init_device();

    init_ramdisk();

#ifdef HAS_CTE
    init_irq();
#endif

    init_fs();

    init_proc();

    Log("Finish initialization");

#ifdef HAS_CTE
    yield();
#endif

    panic("Should not reach here");
}
