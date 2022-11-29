#include "common.h"
#include "syscall.h"

extern char _end;
intptr_t program_break = (intptr_t)&_end;

void sys_write(_Context *c) {
    int fd = c->GPR2;
    char *buf = (char *)c->GPR3;
    size_t count = c->GPR4;
    if (fd == 1 || fd == 2) {
        while (count--) {
            _putc(*buf++);
        }
    }
    c->GPRx = c->GPR4 - count;  //! set return val
}

_Context* do_syscall(_Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;

  switch (a[0]) {
      case SYS_exit: _halt(c->GPR2);  break;
      case SYS_yield: _yield(); c->GPRx = 0; break;
      case SYS_write: sys_write(c); break;
      case SYS_brk: c->GPRx = 0; break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
