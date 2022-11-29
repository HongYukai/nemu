#include "proc.h"
#include <elf.h>

#ifdef __ISA_AM_NATIVE__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

extern uint8_t ramdisk_start;
size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);
size_t get_ramdisk_size();
int fs_open(const char *pathname, int flags, int mode);
size_t fs_read(int fd, void *buf, size_t len);
size_t fs_write(int fd, void *buf, size_t len);
size_t fs_lseek(int fd, size_t offset, int whence);
int fs_close(int fd);
size_t get_disk_offset(int fd);

static uintptr_t loader(PCB *pcb, const char *filename) {
    int fd = fs_open(filename, 0, 0);
    assert(fd != -1);
    size_t disk_offset = get_disk_offset(fd);
    Elf_Ehdr elfheader;
    Elf_Phdr programheader;
    fs_read(fd, &elfheader, sizeof(Elf_Ehdr));
    fs_lseek(fd,elfheader.e_phoff,SEEK_SET);
    for (uint16_t i=0;i<elfheader.e_phnum;i++) {
        fs_read(fd, &programheader, sizeof(Elf_Phdr));
        if (programheader.p_type == PT_LOAD) {
            memmove((void*)programheader.p_vaddr, (void*)(&ramdisk_start + disk_offset + programheader.p_offset), programheader.p_memsz);
            memset((void*)(programheader.p_vaddr + programheader.p_filesz), 0, programheader.p_memsz - programheader.p_filesz);
        }
    }
    fs_close(fd);
    return elfheader.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

void context_kload(PCB *pcb, void *entry) {
  _Area stack;
  stack.start = pcb->stack;
  stack.end = stack.start + sizeof(pcb->stack);

  pcb->cp = _kcontext(stack, entry, NULL);
}

void context_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);

  _Area stack;
  stack.start = pcb->stack;
  stack.end = stack.start + sizeof(pcb->stack);

  pcb->cp = _ucontext(&pcb->as, stack, stack, (void *)entry, NULL);
}
