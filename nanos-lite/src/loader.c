#include <proc.h>
#include <elf.h>
#include <fs.h>

#ifdef __LP64__
#define Elf_Ehdr Elf64_Ehdr
#define Elf_Phdr Elf64_Phdr
#else
#define Elf_Ehdr Elf32_Ehdr
#define Elf_Phdr Elf32_Phdr
#endif

extern size_t ramdisk_read(void *buf, size_t offset, size_t len);
static uintptr_t loader(PCB *pcb, const char *filename)
{
    Elf_Ehdr elf_header;
    int fd = fs_open(filename, 0, 0);
    fs_read(fd, &elf_header, sizeof(Elf_Ehdr));
    // ramdisk_read(&elf_header, 0, sizeof(Elf_Ehdr));
    assert((*(uint32_t *)elf_header.e_ident == 0x464c457f));
#ifdef __ISA_RISCV32__
    assert(elf_header.e_machine == EM_RISCV);
#endif
    int numSegments = elf_header.e_phnum;
    Elf_Phdr *programHeaders =
        (Elf_Phdr *)malloc(sizeof(Elf_Phdr) * numSegments);
    // ramdisk_read(programHeaders, elf_header.e_phoff,
    //              sizeof(Elf_Phdr) * numSegments);
    fs_lseek(fd, elf_header.e_phoff, SEEK_SET);
    fs_read(fd, programHeaders, sizeof(Elf_Phdr) * numSegments);
    for (int i = 0; i < numSegments; i++) {
        if (programHeaders[i].p_type == PT_LOAD) {
            // ramdisk_read((void *)programHeaders[i].p_vaddr,
            //              programHeaders[i].p_offset,
            //              programHeaders[i].p_filesz);
            // //file size or memsize?
            fs_lseek(fd, programHeaders[i].p_offset, SEEK_SET);
            fs_read(fd, (void *)programHeaders[i].p_vaddr,
                    programHeaders[i].p_filesz);
            memset((void *)(programHeaders[i].p_vaddr +
                            programHeaders[i].p_filesz),
                   0, programHeaders[i].p_memsz - programHeaders[i].p_filesz);
        }
    }
    return elf_header.e_entry; // important!
}

void naive_uload(PCB *pcb, const char *filename)
{
    uintptr_t entry = loader(pcb, filename);
    Log("Jump to entry = %p", entry);
    ((void (*)())entry)();
}

void context_uload(PCB *pcb, const char *filename, char *const argv[],
                   char *const envp[])
{
    Area stack;
    stack.start = pcb->stack;
    stack.end = pcb->stack + STACK_SIZE;
    uintptr_t entry = loader(pcb, filename);
    pcb->cp = ucontext(NULL, stack, (void *)entry);

    // argv[0] != filename
    int argc = 0;
    while (argv[argc] != NULL) {
        argc++;
    }
    // argc++;

    int space = 0;
    space += sizeof(int);              // for argc
    space += sizeof(uintptr_t) * argc; // for argv
    if (argv) {
        for (int i = 0; i < argc; ++i) {
            space += strlen(argv[i]) + 1; // for '\0'
            printf("argv: %s\n", argv[i]);
        }
    }
    space += sizeof(uintptr_t); // a null
    // int envpc = 0;
    // if (envp)
    //     while (envp[envpc]) {
    //         envpc++;
    //     }
    // space += sizeof(uintptr_t) * (envpc + 1); // for envp and null
    // if (envp) {
    //     for (int i = 0; i < envpc; ++i)
    //         space += (strlen(envp[i]) + 1);
    // }
    // space += sizeof(uintptr_t); // another null

    uintptr_t *base = stack.end - space * 2; // leave a question
    pcb->cp->GPRx = (uintptr_t)base;
    uintptr_t *args = base;
    *(int *)args = argc;
    char **tmp = (char **)((int *)args + 1);
    char **argv_temp = tmp;
    // memcpy(tmp, argv, argc * sizeof(uintptr_t));
    tmp += argc;
    tmp++;
    *(tmp) = NULL;
    // tmp++;
    // char **envp_temp = tmp;
    // memcpy(tmp, envp, envpc * sizeof(char **));
    // tmp += envpc;
    // tmp++;
    // *(tmp++) = NULL;
    tmp++;
    for (int i = 0; i < argc; ++i) {
        printf("argv: %s\n", argv[i]);
        printf("*s\n", *tmp);
        strcpy(*tmp, argv[i]);
        argv_temp[i] = *tmp;
        *tmp += (strlen(argv[i]) + 1);
    }
}