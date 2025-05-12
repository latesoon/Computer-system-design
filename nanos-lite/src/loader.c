#include "common.h"
#include "fs.h"

#define DEFAULT_ENTRY ((void *)0x8048000)

void ramdisk_read(void *buf, off_t offset, size_t len);
size_t get_ramdisk_size();

void* new_page(void);

uintptr_t loader(_Protect *as, const char *filename) {
  //TODO();
  
  //PA3.1
  //ramdisk_read(DEFAULT_ENTRY,0,get_ramdisk_size());
  //return (uintptr_t)DEFAULT_ENTRY;

  int fd = fs_open(filename, 0, 0);

  //PA3.2
  //fs_read(fd, DEFAULT_ENTRY, fs_filesz(fd));

  int size = fs_filesz(fd);
  for (int sz = 0; sz < size; sz+= PGSIZE){
    void* pa = new_page();
    //Log("map!va:%x pa:%x",DEFAULT_ENTRY + sz, pa);
    _map(as, DEFAULT_ENTRY + sz, pa);
    fs_read(fd, pa, ((size - sz > PGSIZE) ? PGSIZE : (size - sz)));
  }
  fs_close(fd);
  return (uintptr_t)DEFAULT_ENTRY;
}
