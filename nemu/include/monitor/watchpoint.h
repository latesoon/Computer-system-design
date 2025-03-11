#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  uint32_t val;
  char exp[1000];

} WP;

bool wp_check();
void wp_print();
void wp_del(uint32_t no);
void wp_add(char* arg);

#endif
