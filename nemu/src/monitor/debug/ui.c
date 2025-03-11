#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args);

static int cmd_info(char *args);

static int cmd_x(char *args);

static int cmd_p(char *args);

static int cmd_d(char *args){
  char *arg = strtok(args, " ");
  if(arg != NULL)
    wp_del(atoi(arg));
  else
    printf("\033[1;31mMissing arg in cmd d!\033[0m\n");
  return 0;
}

static int cmd_w(char *args){
  if(args == NULL){
    printf("\033[1;31mMissing arg in cmd w!\033[0m\n");
    return 0;
  }
  wp_add(args);
  return 0;
}



static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },

  /* TODO: Add more commands */
  { "si", "Step instruction by 1 or n", cmd_si },
  { "info", "Register info / Watchpoint info", cmd_info },
  { "x", "Get continous data from address", cmd_x },
  { "p", "Calculate expr", cmd_p },
  { "w", "Add watchpoint", cmd_w },
  { "d", "Delete watchpoint", cmd_d }
};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

static int cmd_si(char *args) {
  uint64_t step = 1;
  char *arg = strtok(args, " ");
  if(arg != NULL)
    step = atoi(arg);
  cpu_exec(step);
  return 0;
}

static int cmd_info(char *args) {
  char *arg = strtok(args, " ");
  if(arg == NULL){
    printf("\033[1;31mMissing arg in cmd info!\033[0m\n");
    return 0;
  }
  if(!strcmp(strtok(args," "),"r")){
    printf("Registers info:\n");
    printf("EAX:0x%08X ECX:0x%08X\n",cpu.eax,cpu.ecx);
    printf("EDX:0x%08X EBX:0x%08X\n",cpu.edx,cpu.ebx);
    printf("ESP:0x%08X EBP:0x%08X\n",cpu.esp,cpu.ebp);
    printf("ESI:0x%08X EDI:0x%08X\n",cpu.esi,cpu.edi);
    printf("EIP:0x%08X\n",cpu.eip);
    return 0;
  }
  if(!strcmp(strtok(args," "),"w")){
    printf("Watchpoints info:\n");
    wp_print();
    return 0;
  }
  printf("\033[1;31mInvalid arg in cmd info!\033[0m\n");
  return 0;
}

static int cmd_x(char *args){
  char *arg1 = strtok(args, " ");
  //char *arg2 = strtok(NULL, " ");
  char *arg2 = args + strlen(arg1) + 1;
  if(arg1 == NULL || arg2 == NULL){
    printf("\033[1;31mMissing arg in cmd x!\033[0m\n");
    return 0;
  }
  if(atoi(arg1)<=0){
    printf("\033[1;31mInvalid arg in cmd x!\033[0m\n");
    return 0;
  }
  uint64_t len = atoi(arg1);
  //uint32_t addr = strtoul(arg2,NULL,16);
  bool succ = true;
  uint32_t addr = expr(arg2, &succ);
  if(!succ){
    printf("\033[1;31mInvalid arg2 in cmd x!\033[0m\n");
    return 0;
  }
  printf("Address     Data\n");
  while(len){
    int cnt = 8;
    printf("0x%08X: ",addr);
    while(cnt && len){
      printf("%02X ",vaddr_read(addr,1));
      addr++,cnt--,len--;
    }
    printf("\n");
  }
  return 0;
}

static int cmd_p(char *args){
  if(args == NULL){
    printf("\033[1;31mMissing arg in cmd p!\033[0m\n");
    return 0;
  }
  bool succ = true;
  uint32_t val = expr(args, &succ);
  if(!succ){
    printf("\033[1;31mInvalid arg in cmd p!\033[0m\n");
    return 0;
  }
  printf("The answer is:\n");
  printf("Base10:%d\n",val);
  printf("Base16:0x%08X\n",val);
  return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
