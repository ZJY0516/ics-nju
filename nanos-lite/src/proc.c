#include <proc.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;
extern void naive_uload(PCB *pcb, const char *filename);
extern void context_uload(PCB *pcb, const char *filename, char *const argv[],
                          char *const envp[]);

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
        if (j % 10000 == 0) {
            Log("Hello World from Nanos-lite with arg '%s' for the %dth time!",
                (char *)arg, j / 10000);
        }
        j++;
        yield();
    }
}

void init_proc()
{
    // char *pal_argv[] = {"--skip", "argv1", "argv2", NULL};
    // context_kload(&pcb[0], hello_fun, "114");
    // context_uload(&pcb[1], "/bin/nterm", pal_argv, NULL);
    // switch_boot_pcb();

    Log("Initializing processes...");

    // load program here
    naive_uload(NULL, "/bin/menu"); // menu or menu? hhh
}

Context *schedule(Context *prev)
{
    current->cp = prev;
    current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
    return current->cp;
}
