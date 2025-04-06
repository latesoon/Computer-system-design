#include <am.h>
#include <x86.h>

#define RTC_PORT 0x48   // Note that this is not standard
static unsigned long boot_time;

void _ioe_init() {
  boot_time = inl(RTC_PORT);
}

unsigned long _uptime() {
  //return 0;
  return inl(RTC_PORT) - boot_time;
}

uint32_t* const fb = (uint32_t *)0x40000;

_Screen _screen = {
  .width  = 400,
  .height = 300,
};

extern void* memcpy(void *, const void *, int);

void _draw_rect(const uint32_t *pixels, int x, int y, int w, int h) {
  //int i;
  //for (i = 0; i < _screen.width * _screen.height; i++) {
  //  fb[i] = i;
  //}
  //assert( w+x <= _screen.width && (y+h) <= _screen.height);
  for(int i = y, j = 0; i < y + h ; i++,j++)
    memcpy(fb + i * _screen.width + x , pixels + j * w, sizeof(uint32_t) * w);
}

void _draw_sync() {
}

#define KEYBOARD_DATA 0x60
#define KEYBOARD_STATUS 0x64

int _read_key() {
  if(inb(KEYBOARD_STATUS))
    return inl(KEYBOARD_DATA);
  else
    return _KEY_NONE;
}
