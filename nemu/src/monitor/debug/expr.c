#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

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

uint32_t eval(uint32_t start, uint32_t end, bool* succ){
  return 0;
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
