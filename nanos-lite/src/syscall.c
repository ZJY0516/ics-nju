#include <common.h>
#include <sys/time.h>
#include "syscall.h"
#include <fs.h>
#include <proc.h>
#include "../../nemu/include/generated/autoconf.h"
static intptr_t heap_brk;
extern void naive_uload(PCB *pcb, const char *filename);
int sys_execve(const char *pathname, char *const argv[], char *const envp[])
{
    // 1 stands for failure, no such file
    // if (fs_open(pathname, 0, 0) == -1) {
    //     return 1;
    // }
    naive_uload(NULL, pathname);
    return -1;
}

void do_syscall(Context *c)
{
    uintptr_t a[4];
    a[0] = c->GPR1;
    uintptr_t call_para[3];
    call_para[0] = c->GPR2;
    call_para[1] = c->GPR3;
    call_para[2] = c->GPR4;

    switch (a[0]) {
    case SYS_execve:
        c->GPRx =
            sys_execve((const char *)call_para[0], (char *const *)call_para[1],
                       (char *const *)call_para[2]);
        break;
    case SYS_exit:
        // printf("code:%d\n", (int)call_para[0]);
        if (call_para[0] == 0) {
            sys_execve("/bin/nterm", NULL, NULL);
        } else
            halt(call_para[0]); // i don't understand
        break;
    case SYS_yield:
        yield();
        c->GPRx = 0;
        break;
    case SYS_write:
        // if (call_para[0] == 1 || call_para[0] == 2) {
        //     for (int i = 0; i < call_para[2]; i++) {
        //         putch(*(char *)(call_para[1] + i));
        //     }
        //     c->GPRx = call_para[2];
        //     break;
        // }
        c->GPRx =
            fs_write(call_para[0], (const void *)call_para[1], call_para[2]);
        break;
    case SYS_brk:
        heap_brk = (intptr_t)call_para[0];
        c->GPRx = 0;
        break;
    case SYS_open:
        c->GPRx = fs_open((char *)call_para[0], call_para[1], call_para[2]);
        break;
    case SYS_lseek:
        c->GPRx = fs_lseek((int)call_para[0], (size_t)call_para[1],
                           (int)call_para[2]);
        break;
    case SYS_close:
        c->GPRx = fs_close((int)call_para[0]);
        break;
    case SYS_read:
        c->GPRx = fs_read(call_para[0], (void *)call_para[1], call_para[2]);
        break;
    case SYS_gettimeofday:
        struct timeval *tv = (struct timeval *)call_para[0];
        AM_TIMER_UPTIME_T uptime;
        ioe_read(AM_TIMER_UPTIME, &uptime);
        tv->tv_usec = (int32_t)uptime.us % 1000000;
        tv->tv_sec = (int32_t)uptime.us / 1000000;
        // printf("size of tv->sec %d\n", sizeof(tv->tv_sec));
        c->GPRx = 0;
        break;
    default:
        panic("Unhandled syscall ID = %d", a[0]);
    }
#ifdef CONFIG_STRACE
    Log("syscall ID = %d  return value = %d", a[0], c->GPRx);
#endif
}
