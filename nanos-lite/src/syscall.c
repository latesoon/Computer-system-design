#include "common.h"
#include "syscall.h"

int do_syswrite(int fd,const void* buf,size_t len){
  if(fd == 1 || fd == 2){
    for(int i=0;i<len;i++)
      _putc(((char*)(buf))[i]);
  }
  //printf("%d %d\n",fd,len);
  return len;
}

_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);
  a[1] = SYSCALL_ARG2(r);
  a[2] = SYSCALL_ARG3(r);
  a[3] = SYSCALL_ARG4(r);
  
  switch (a[0]) {
    case SYS_none:
      SYSCALL_ARG1(r) = 1;
      break;
    case SYS_exit:
      _halt(SYSCALL_ARG2(r));
      break; 
    case SYS_write:
      Log("SYS_write!");
      SYSCALL_ARG1(r) = do_syswrite(((int)(a[1])), ((char*)(a[2])),((size_t)(a[3])));
      break;
    case SYS_brk:
      SYSCALL_ARG1(r) = 0;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
