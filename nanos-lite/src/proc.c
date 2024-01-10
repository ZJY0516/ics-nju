#include <proc.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;
extern void naive_uload(PCB *pcb, const char *filename);
extern void context_uload(PCB *pcb, const char *filename);

void context_kload(PCB *pcb, void (*entry)(void *), void *arg)
{
    Area kstack;
    kstack.start = pcb->stack;
    kstack.end = pcb->stack + STACK_SIZE;
    pcb->cp = kcontext(kstack, entry, arg);
}

void switch_boot_pcb() { current = &pcb_boot; }

void hello_fun(void *arg)
{
    int j = 1;
    while (1) {
        assert(j < 10);
        for (const char *p = (char *)arg; *p; p++)
            putch(*p);
        printf("arg: %d\n", (int)arg);
        // printf("arg-4: %s\n", (arg - 4));
        for (const char *p = (char *)arg; *p; p++)
            putch(*p);
        printf("Hello World from Nanos-lite with arg '%s' for the %dth time!",
               (char *)arg, j);
        // for (const char *p = (char *)arg; *p; p++)
        //     putch(*p);
        putch('\n');
        j++;
        yield();
    }
}

void init_proc()
{
    context_kload(&pcb[0], hello_fun, "114");
    context_kload(&pcb[1], hello_fun, "514");
    switch_boot_pcb();

    Log("Initializing processes...");

    // load program here
    // naive_uload(NULL, "/bin/menu"); // menu or menu? hhh
}

Context *schedule(Context *prev)
{
    current->cp = prev;
    current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
    return current->cp;
}
