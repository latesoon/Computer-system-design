#include "common.h"
#include "syscall.h"
#include "fs.h"

extern int mm_brk(uint32_t new_brk);

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
      //PA3.1
      //Log("SYS_write!");
      //SYSCALL_ARG1(r) = do_syswrite(((int)(a[1])), ((char*)(a[2])),((size_t)(a[3])));
      //break;

      //PA3.2
      SYSCALL_ARG1(r) = fs_write(((int)(a[1])), ((char*)(a[2])),((size_t)(a[3])));
      break;
    case SYS_brk:
      //SYSCALL_ARG1(r) = 0;
      SYSCALL_ARG1(r) = mm_brk((uint32_t)(a[2]));
      break;
    case SYS_read:
      SYSCALL_ARG1(r) = fs_read(((int)(a[1])), ((char*)(a[2])),((size_t)(a[3])));
      break;
    case SYS_lseek:
      SYSCALL_ARG1(r) = fs_lseek(((int)(a[1])), ((off_t)(a[2])),((int)(a[3])));
      break;
    case SYS_open:
      SYSCALL_ARG1(r) = fs_open(((char*)(a[1])), ((int)(a[2])),((int)(a[3])));
      break;
    case SYS_close:
      SYSCALL_ARG1(r) = fs_close((int)(a[1]));
      break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
