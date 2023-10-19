#include <common.h>
#include <elf.h>

#define MAX_I_RING_BUF_LEN 16
#define MAX_FUNC_LIST 10240

typedef struct {
    word_t pc;
    word_t inst;
} instruction;

typedef struct {
    char name[64]; // 24 mayb enough
    paddr_t start;
    size_t size;
} func;

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

void show_pread(paddr_t addr, int len)
{
#ifdef CONFIG_MTRACE
    log_write("pread " FMT_PADDR "len=%d\n", addr, len);
#endif
}
void show_pwrite(paddr_t addr, int len)
{
#ifdef CONFIG_MTRACE
    log_write("pwrite " FMT_PADDR "len=%d\n", addr, len);
#endif
}

#ifdef CONFIG_FTRACE
func func_list[MAX_FUNC_LIST]; // linked list is more suitable
size_t func_cnt = 0;
#endif
static void collect_func(FILE *elf_fp)
{
#ifdef CONFIG_FTRACE
    Elf32_Ehdr elf_header;
    if (fread(&elf_header, sizeof(Elf32_Ehdr), 1, elf_fp) != 1) {
        perror("Failed to read ELF header");
        fclose(elf_fp);
        return;
    }
    // read the section header table
    fseek(elf_fp, elf_header.e_shoff, SEEK_SET);
    Elf32_Shdr *section_headers =
        (Elf32_Shdr *)malloc(sizeof(Elf32_Shdr) * elf_header.e_shnum);
    if (!fread(section_headers, sizeof(Elf32_Shdr), elf_header.e_shnum,
               elf_fp)) {
        //! stands for not in c, !! is a plugin(sudo) shortcut in omz
        // annoying
        // Failed to read section header table: Operation not permitted
        perror("Failed to read section header table");
        free(section_headers);
        fclose(elf_fp);
        return;
    }
    int symbol_table_index = -1;
    int string_table_index = -1;

    for (int i = 0; i < elf_header.e_shnum; i++) {
        if (section_headers[i].sh_type == SHT_SYMTAB) {
            symbol_table_index = i;
        } else if (section_headers[i].sh_type == SHT_STRTAB) {
            // 区分字符串表和节头字符串表
            if (i != elf_header.e_shstrndx) {
                string_table_index = i;
            }
        }
    }
    // Read symbol table
    fseek(elf_fp, section_headers[symbol_table_index].sh_offset, SEEK_SET);
    Elf32_Sym *symbol_table =
        (Elf32_Sym *)malloc(section_headers[symbol_table_index].sh_size);
    if (fread(symbol_table, section_headers[symbol_table_index].sh_size, 1,
              elf_fp) != 1) {
        perror("Failed to read symbol table");
        free(symbol_table);
        free(section_headers);
        fclose(elf_fp);
        return;
    }
    // Read string table
    fseek(elf_fp, section_headers[string_table_index].sh_offset, SEEK_SET);
    char *string_table =
        (char *)malloc(section_headers[string_table_index].sh_size);
    if (fread(string_table, section_headers[string_table_index].sh_size, 1,
              elf_fp) != 1) {
        perror("Failed to read string table");
        free(string_table);
        free(symbol_table);
        free(section_headers);
        fclose(elf_fp);
        return;
    }
    assert(section_headers[symbol_table_index].sh_size / sizeof(Elf32_Sym) <
           MAX_FUNC_LIST);
    for (int i = 0;
         i < section_headers[symbol_table_index].sh_size / sizeof(Elf32_Sym);
         i++) {
        if (ELF32_ST_TYPE(symbol_table[i].st_info) == STT_FUNC) {
            strcpy(func_list[func_cnt].name,
                   &string_table[symbol_table[i].st_name]);
            func_list[func_cnt].start = symbol_table[i].st_value;
            func_list[func_cnt].size = symbol_table[i].st_size;
            func_cnt++;
        }
    }
    // Clean up
    free(string_table);
    free(symbol_table);
    free(section_headers);
    fclose(elf_fp);
#endif
}
int call_depth = 0;
int ret_depth = 0;
// int depth = 0;
void ftrace(paddr_t pc, paddr_t dnpc, bool is_call)
{
#ifdef CONFIG_FTRACE
    int depth;
    if (is_call)
        depth = call_depth;
    else
        depth = ret_depth;
    int i;
    for (i = 0; i < func_cnt; i++) {
        if (dnpc >= func_list[i].start &&
            dnpc < (func_list[i].start + func_list[i].size))
            break;
    }
    log_write(FMT_PADDR ": ", pc);
    for (int x = 0; x < depth; x++)
        log_write("  ");
    if (is_call) {
        ret_depth = depth;
        log_write("call [%s@" FMT_PADDR "]\n", func_list[i].name, dnpc);
        call_depth++;
    } else {
        log_write("ret [%s]\n", func_list[i].name);
        call_depth--;
        ret_depth--;
    }
#endif
}
void init_elf(const char *elf_file)
{
#ifdef CONFIG_TRACE
    if (elf_file != NULL) {
        FILE *elf = fopen(elf_file, "rb");
        Assert(elf, "Can not open '%s'", elf_file);
        collect_func(elf);
    } else {
        printf("elf file is null\n");
    }
#endif
}
