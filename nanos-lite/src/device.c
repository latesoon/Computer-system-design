#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

size_t events_read(void *buf, size_t len) {
  int key = _read_key();
  if(key == _KEY_NONE)
    sprintf(buf,"t %d\n",_uptime());
  else
    sprintf(buf,"%s %s\n",(key & 0x8000) ? "kd" : "ku", keyname[key & 255]);
  return strlen(buf);
}

static char dispinfo[128] __attribute__((used));

void dispinfo_read(void *buf, off_t offset, size_t len) {
  strncpy(buf, dispinfo+offset, len);
}

void fb_write(const void *buf, off_t offset, size_t len) {
  assert(!(offset%sizeof(uint32_t) | len%sizeof(uint32_t)));
  _draw_rect((uint32_t *)buf, offset/sizeof(uint32_t)%_screen.width, offset/sizeof(uint32_t)/_screen.width, len/sizeof(uint32_t),1);
}

void init_device() {
  _ioe_init();

  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention

  sprintf(dispinfo, "WIDTH:%d\nHEIGHT:%d",_screen.width,_screen.height);
}
