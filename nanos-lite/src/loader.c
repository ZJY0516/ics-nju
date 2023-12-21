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
