#include <am.h>
#include <nemu.h>
#include <klib.h>

static AddrSpace kas = {};
static void* (*pgalloc_usr)(int) = NULL;
static void (*pgfree_usr)(void*) = NULL;
static int vme_enable = 0;

static Area segments[] = {      // Kernel memory mappings
  NEMU_PADDR_SPACE
};

#define USER_SPACE RANGE(0x40000000, 0x80000000)

static inline void set_satp(void *pdir) {
  uintptr_t mode = 1ul << (__riscv_xlen - 1);
  asm volatile("csrw satp, %0" : : "r"(mode | ((uintptr_t)pdir >> 12)));
}

static inline uintptr_t get_satp() {
  uintptr_t satp;
  asm volatile("csrr %0, satp" : "=r"(satp));
  return satp << 12;
}

bool vme_init(void* (*pgalloc_f)(int), void (*pgfree_f)(void*)) {
  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  kas.ptr = pgalloc_f(PGSIZE);

  int i;
  for (i = 0; i < LENGTH(segments); i ++) {
    void *va = segments[i].start;
    for (; va < segments[i].end; va += PGSIZE) {
      map(&kas, va, va, 0);
    }
  }

  set_satp(kas.ptr);
  vme_enable = 1;

  return true;
}

void protect(AddrSpace *as) {
  PTE *updir = (PTE*)(pgalloc_usr(PGSIZE));
  as->ptr = updir;
  as->area = USER_SPACE;
  as->pgsize = PGSIZE;
  // map kernel space
  memcpy(updir, kas.ptr, PGSIZE);
}

void unprotect(AddrSpace *as) {
}

void __am_get_cur_as(Context *c) {
  c->pdir = (vme_enable ? (void *)get_satp() : NULL);
}

void __am_switch(Context *c) {
  if (vme_enable && c->pdir != NULL) {
    set_satp(c->pdir);
  }
}

#define VPN1(va) (((uintptr_t)va >> 22) & 0x000003FF)
#define VPN0(va) (((uintptr_t)va >> 12) & 0x000003FF)
#define PPN(pte) ((pte) >> 12)

void map(AddrSpace *as, void *va, void *pa, int prot)
{
    assert(as);
    // assert(as->pgsize == PGSIZE);

    uintptr_t *pte1 =
        (uintptr_t *)((uintptr_t)as->ptr + VPN1(va) * sizeof(uintptr_t));
    if ((*pte1 & 0x1) == 0) {
        uintptr_t newpage = (uintptr_t)pgalloc_usr(PGSIZE);
        *pte1 = (newpage >> 2) | 1;
    }
    uintptr_t *pte0 = (uintptr_t *)(PPN(*pte1) + VPN0(va) * sizeof(uintptr_t));

    *pte0 = ((uintptr_t)pa >> 2) | 0xf;
}

Context *ucontext(AddrSpace *as, Area kstack, void *entry)
{
    Context *c = kstack.end - sizeof(Context);
    *c = (Context){0};
    c->mepc = (uintptr_t)entry;
    c->mstatus = 0x1800;
    return c;
}
