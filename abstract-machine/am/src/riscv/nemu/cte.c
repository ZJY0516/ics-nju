#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

static Context *(*user_handler)(Event, Context *) = NULL;

Context *__am_irq_handle(Context *c)
{
    // printf("event id: %d\n", (int)c->mcause);
    // printf("yield: %d\n", (int)c->gpr[17]);
    if (user_handler) {
        Event ev = {0};
        switch (c->mcause) {
        case 0xb:
            if (c->GPR1 == -1) {
                ev.event = EVENT_YIELD;
            } else {
                ev.event = EVENT_SYSCALL;
            } // how to choose event through mcause
            c->mepc += 4;
            break;
        default:
            ev.event = EVENT_ERROR;
            // ev.event = EVENT_SYSCALL; // so just distiniguish yield and
            // syscall?
            break;
        }

        c = user_handler(ev, c);
        assert(c != NULL);
    }

    return c;
}

extern void __am_asm_trap(void);

bool cte_init(Context *(*handler)(Event, Context *))
{
    // initialize exception entry
    asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));

    // register event handler
    user_handler = handler;

    return true;
}

Context *kcontext(Area kstack, void (*entry)(void *), void *arg)
{
    printf("arg: %s\n", (char *)arg);
    Context *c = kstack.end - sizeof(Context);
    *c = (Context){0};
    c->mepc = (uintptr_t)entry;
    c->mstatus = 0x1800;
    c->GPR2 = (uintptr_t)arg;
    return c;
}

void yield()
{
#ifdef __riscv_e
    asm volatile("li a5, -1; ecall");
#else
    asm volatile("li a7, -1; ecall");
#endif
}

bool ienabled() { return false; }

void iset(bool enable) {}
