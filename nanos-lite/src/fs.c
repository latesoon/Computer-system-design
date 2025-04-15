#include "fs.h"

typedef struct {
  char *name;
  size_t size;
  off_t disk_offset;
  off_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO, FD_NORMAL};

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin (note that this is not the actual stdin)", 0, 0},
  {"stdout (note that this is not the actual stdout)", 0, 0},
  {"stderr (note that this is not the actual stderr)", 0, 0},
  [FD_FB] = {"/dev/fb", 0, 0},
  [FD_EVENTS] = {"/dev/events", 0, 0},
  [FD_DISPINFO] = {"/proc/dispinfo", 128, 0},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

void init_fs() {
  // TODO: initialize the size of /dev/fb
}

size_t fs_filesz(int fd){
  return file_table[fd].size;
}

static inline off_t tot_offset(int fd){
  return file_table[fd].disk_offset + file_table[fd].open_offset;
}

static inline off_t fs_offset(int fd){
  return file_table[fd].open_offset;
}

static inline off_t update_offset3(int fd, int len, int mode){
  Log("fd%d len%d mode%d size%d",fd,len,mode,fs_filesz(fd));
  if(mode == SEEK_SET)
    file_table[fd].open_offset = 0;
  else if(mode == SEEK_END)
    file_table[fd].open_offset = file_table[fd].size;
  Log(" newoffset %d\n",fs_offset(fd));
  file_table[fd].open_offset += len;
  file_table[fd].open_offset = ((file_table[fd].open_offset > len) ? len : file_table[fd].open_offset);
  file_table[fd].open_offset = ((file_table[fd].open_offset < 0) ? 0 : file_table[fd].open_offset);
  Log(" newoffset %d\n",fs_offset(fd));
  return file_table[fd].open_offset;
}

static inline void update_offset(int fd, int len){
  update_offset3(fd, len, SEEK_CUR);
}

int fs_open(const char* pathname, int flags, int mode){
  //Log("%s:%d\n",pathname,NR_FILES);
  for(int fd = 0; fd < NR_FILES; fd++){
    if(!strcmp(pathname,file_table[fd].name))
      return fd;
  }
  assert(0);//should not reach here
  return -1;
}

void ramdisk_read(void *buf, off_t offset, size_t len);

ssize_t fs_read(int fd, void* buf, size_t len){
  Log("%d:size %d,len %d,offset %d",fd,fs_filesz(fd),len,fs_offset(fd));
  switch(fd){
    case FD_STDIN:
    case FD_STDOUT:
    case FD_STDERR:
      break;
    default:
      len = (fs_filesz(fd) - fs_offset(fd) >= len) ? len : (fs_filesz(fd) - fs_offset(fd));
      if(len <= 0) 
        return 0;
      ramdisk_read(buf, tot_offset(fd), len);
      update_offset(fd, len);
  }
  Log(" newoffset %d\n",fs_offset(fd));
  return len;
}

int fs_close(int fd){
  return 0;
}

off_t fs_lseek(int fd, off_t offset, int whence){
  assert(whence == SEEK_CUR || whence == SEEK_END || whence == SEEK_SET);
  return update_offset3(fd, offset, whence);
}

void ramdisk_write(const void *buf, off_t offset, size_t len);

ssize_t fs_write(int fd, const void* buf, size_t len){
  Log("%d:size %d,len %d,offset %d",fd,fs_filesz(fd),len,fs_offset(fd));
  switch(fd){
    case FD_STDIN:
      break;
    case FD_STDOUT:
    case FD_STDERR:
      for(int i=0;i<len;i++)
      _putc(((char*)(buf))[i]);
      break;
    default:
      len = (fs_filesz(fd) - fs_offset(fd) >= len) ? len : (fs_filesz(fd) - fs_offset(fd));
      if(len <= 0) 
        return 0;
      ramdisk_write(buf, tot_offset(fd), len);
      update_offset(fd, len);
  }
  Log(" newoffset %d\n",fs_offset(fd));
  return len;
}