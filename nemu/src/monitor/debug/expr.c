#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>

enum {
  TK_NOTYPE = 256, TK_EQ,

  /* TODO: Add more token types */

  TK_NEQ,TK_AND,TK_OR,TK_NUM,TK_HEX,TK_REG,TK_UNARYPLUS,TK_UNARYSUB,TK_DEREF

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ},        // equal

  {"\\-", '-'},
  {"\\*", '*'},
  {"/", '/'},
  {"\\(", '('},
  {"\\)", ')'},
  {"!=",TK_NEQ},
  {"&&",TK_AND},
  {"\\|\\|",TK_OR},
  {"\\$[a-zA-Z0-9]+",TK_REG},
  {"0[xX][0-9a-fA-F]+",TK_HEX},
  {"0|[1-9][0-9]*",TK_NUM},
  {"!",'!'}
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[32];
int nr_token;

int priority[300];

static bool make_token(char *e) {

  priority[TK_NUM] = priority[TK_HEX] = priority[TK_REG] = 1;
  priority['('] = priority[')'] = 2;
  priority[TK_UNARYPLUS] = priority[TK_UNARYSUB] = priority['!'] = priority[TK_DEREF] = 3;
  priority['*'] = priority['/'] = 4;
  priority['+'] = priority['-'] = 5;
  priority[TK_EQ] = priority[TK_NEQ] = 6;
  priority[TK_AND] = 7;
  priority[TK_OR] = 8;
  

  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_NOTYPE:
            break;
          case '+':
            if(!nr_token || (tokens[nr_token-1].type !=')' && tokens[nr_token-1].type != TK_NUM 
              && tokens[nr_token-1].type != TK_HEX && tokens[nr_token-1].type != TK_REG)){
              tokens[nr_token].type = TK_UNARYPLUS; 
            }
            else{
              tokens[nr_token].type = rules[i].token_type;
            }
            nr_token++;
            break;
          case '-':
            if(!nr_token || (tokens[nr_token-1].type !=')' && tokens[nr_token-1].type != TK_NUM 
              && tokens[nr_token-1].type != TK_HEX && tokens[nr_token-1].type != TK_REG)){
              tokens[nr_token].type = TK_UNARYSUB; 
            }
            else{
              tokens[nr_token].type = rules[i].token_type;
            }
            nr_token++;
            break;
          case '*':
            if(!nr_token || (tokens[nr_token-1].type !=')' && tokens[nr_token-1].type != TK_NUM 
              && tokens[nr_token-1].type != TK_HEX && tokens[nr_token-1].type != TK_REG)){
              tokens[nr_token].type = TK_DEREF; 
            }
            else{
              tokens[nr_token].type = rules[i].token_type;
            }
            nr_token++;
            break;
          case TK_HEX:
          case TK_NUM:
          case TK_REG:
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len]='\0';
          default: //TODO();
            tokens[nr_token].type = rules[i].token_type;
            nr_token++;
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

bool check_par(uint32_t s,uint32_t e){
  if(tokens[s].type !='(' ||tokens[e].type !=')')
    return false;
  int cnt = 0;
  for(int now = s; now <= e ; now++){
    if(tokens[now].type =='(')
      cnt++;
    else if(tokens[now].type ==')'){
      cnt--;
      if(cnt == 0 && now!= e)
        return false;
    }
  }
  if(cnt) return false;
  return true;
}

uint32_t eval(uint32_t s, uint32_t e, bool* succ){
  if(!(*succ))
    return 0;
  if(s > e){
    *succ = false;
    return 0;
  }
  else if(s == e){
    char reg[4];
    switch(tokens[s].type){
      case TK_HEX:
        return strtoul(tokens[s].str,NULL,16);
      case TK_NUM:
        return strtoul(tokens[s].str,NULL,10);
      case TK_REG:
        for(int i=1;i<=4;i++){
          if(tokens[s].str[i] >= 'A' && tokens[s].str[i] <= 'Z')
            reg[i-1] = tokens[s].str[i] + 32;
          else if(tokens[s].str[i] >= 'a' && tokens[s].str[i] <= 'z')
            reg[i-1] = tokens[s].str[i];
          else if(tokens[s].str[i] == '\0'){
            reg[i-1] = tokens[s].str[i];
            break;
          }
          else{
            *succ = false;
            return 0; 
          }
        }
        for(int i=0;i<8;i++){
          if(!strcmp(reg,regsl[i]))
            return cpu.gpr[i]._32;
          if(!strcmp(reg,regsw[i]))
            return cpu.gpr[i]._16;
          if(!strcmp(reg,regsb[i]))
            return cpu.gpr[i%4]._8[i/4];
        }
        if(!strcmp(reg,"eip"))
          return cpu.eip;
      //all not match, also into default
      default:
        *succ = false;
        return 0; 
    }
  }
  else if(check_par(s,e))
    return eval(s+1,e-1,succ);
  else{
    int par = 0;
    int mon = -1;
    for(int now=s;now<=e;now++){
      switch(tokens[now].type){
        case '(':
          par++;
          break;
        case ')':
          par--;
          break;
        default:
          if(par)
            break;
          if(mon == -1 || (priority[tokens[now].type] >= priority[tokens[mon].type] && priority[tokens[now].type] >3)
            || priority[tokens[now].type] > priority[tokens[mon].type])
            mon = now;
      }
    }
    if(par){
      *succ = false;
      return 0; 
    }
    if(priority[tokens[mon].type] == 3){
      uint32_t val = eval(mon+1,e,succ);
      switch(tokens[mon].type){
        case TK_UNARYPLUS: 
          return val;
        case TK_UNARYSUB: 
          return -val;
        case '!': 
          return !val;
        case TK_DEREF: 
          return vaddr_read(val,4);
        default:
          *succ = false;
          return 0; 
      }
    }
    else{
      uint32_t val1 = eval(s,mon-1,succ);
      uint32_t val2 = eval(mon+1,e,succ);
      switch(tokens[mon].type){
        case TK_AND:
          return val1 && val2;
        case TK_OR:
          return val1 || val2;
        case TK_EQ:
          return val1 == val2;
        case TK_NEQ:
          return val1 != val2;
        case '+':
          return val1 + val2;
        case '-':
          return val1 - val2;
        case '*':
          return val1 * val2;
        case '/':
          return val1 / val2;
        default:
          *succ = false;
          return 0; 
      }
    } 
  }
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  //TODO();
  return eval(0,nr_token-1,success);
  //return 0;
}
