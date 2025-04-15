#include "common.h"
#include "fs.h"

#define DEFAULT_ENTRY ((void *)0x4000000)

void ramdisk_read(void *buf, off_t offset, size_t len);
size_t get_ramdisk_size();

uintptr_t loader(_Protect *as, const char *filename) {
  //TODO();
  
  //PA3.1
  //ramdisk_read(DEFAULT_ENTRY,0,get_ramdisk_size());

  //PA3.2
  int fd = fs_open(filename, 0, 0);
  fs_read(fd, DEFAULT_ENTRY, fs_filesz(fd));
  fs_close(fd);
  return (uintptr_t)DEFAULT_ENTRY;
}
