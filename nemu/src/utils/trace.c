#include <common.h>

#define MAX_I_RING_BUF_LEN 16

typedef struct {
    word_t pc;
    word_t inst;
} instruction;

#ifdef CONFIG_ITRACE
int inst_index = 0;
bool iringbuf_is_full = false;

instruction iringbuf[MAX_I_RING_BUF_LEN];
#endif
void add_inst_trace(word_t pc, word_t inst)
{
#ifdef CONFIG_ITRACE
    iringbuf[inst_index].pc = pc;
    iringbuf[inst_index].inst = inst;
    inst_index = (inst_index + 1) % MAX_I_RING_BUF_LEN;
    if (inst_index == 0) {
        iringbuf_is_full = true;
    }
#endif
}
void show_iringbuf()
{
#ifdef CONFIG_ITRACE
    int i;
    char buf[32];
    char *p;
    p = buf;
    void disassemble(char *str, int size, uint64_t pc, uint8_t *code,
                     int nbyte);
    for (i = 0; i < MAX_I_RING_BUF_LEN; i++) {
        disassemble(p, sizeof(buf), iringbuf[i].pc,
                    (uint8_t *)&iringbuf[i].inst, 4);
        if (i == (inst_index - 1 + MAX_I_RING_BUF_LEN) % MAX_I_RING_BUF_LEN) {
            if (!iringbuf_is_full) {
                printf("-->" FMT_WORD "%-20s %08x\n", iringbuf[i].pc, buf,
                       iringbuf[i].inst);
                break;
            }
        } else {
            printf("   " FMT_WORD "%-20s %08x\n", iringbuf[i].pc, buf,
                   iringbuf[i].inst);
        }
    }
#endif
}
