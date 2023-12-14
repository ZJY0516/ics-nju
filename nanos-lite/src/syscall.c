#include <common.h>
#include "syscall.h"
#include <fs.h>
#include "../../nemu/include/generated/autoconf.h"
void do_syscall(Context *c)
{
    uintptr_t a[4];
    a[0] = c->GPR1;
    uintptr_t call_para[3];
    call_para[0] = c->GPR2;
    call_para[1] = c->GPR3;
    call_para[2] = c->GPR4;

    switch (a[0]) {
    case SYS_exit:
        halt(c->GPR2); // i don't understand
        break;
    case SYS_yield:
        yield();
        c->GPRx = 0;
        break;
    case SYS_write:
        if (call_para[0] == 1) {
            for (int i = 0; i < call_para[2]; i++) {
                putch(*(char *)(call_para[1] + i));
            }
            c->GPRx = call_para[2];
            break;
        }
        c->GPRx =
            fs_write(call_para[0], (const void *)call_para[1], call_para[2]);
        break;
    case SYS_brk:
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
    default:
        panic("Unhandled syscall ID = %d", a[0]);
    }
#ifdef CONFIG_STRACE
    Log("syscall ID = %d  return value = %d", a[0], c->GPRx);
#endif
}
