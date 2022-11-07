#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_NUM, TK_HEX,TK_REG, TK_DEREF,

  /* TODO: Add more token types */

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {"\\s+", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ},         // equal
  {"\\*", '*'},
  {"\\/", '/'},
  {"\\-", '-'},
  {"\\(", '('},
  {"\\)", ')'},
  {"0x[0-9a-fA-F]+", TK_HEX},
  {"[0-9]+", TK_NUM},
  {"^\\$[a-z]+", TK_REG}

};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX] = {};

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

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
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
        char sub_str[32];
        snprintf(sub_str, substr_len + 1, "%s", substr_start);

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
            case TK_NOTYPE: {
                break;
            }
            case TK_HEX: {
                tokens[nr_token].type = rules[i].token_type;
                int t;
                sscanf(sub_str, "%x", &t);
                snprintf(tokens[nr_token].str, 32, "%d", t);
                nr_token++;
                break;
            }
            case TK_REG: {
                tokens[nr_token].type = rules[i].token_type;
                bool success;
                extern uint32_t isa_reg_str2val(const char*, bool*);
                uint32_t t = isa_reg_str2val(sub_str, &success);
                assert(success == true);
                snprintf(tokens[nr_token].str, 32, "%d", t);
                nr_token++;
                break;
            }
            case '*': {
                if (rules[i].token_type == '*' && (nr_token == 0 || tokens[nr_token - 1].type == '+' || tokens[nr_token - 1].type == '*' || tokens[nr_token - 1].type == '-' || tokens[nr_token - 1].type == '/') ) {
                    tokens[nr_token].type = TK_DEREF;
                }
                else {
                    tokens[nr_token].type = rules[i].token_type;
                }
                strcpy(tokens[nr_token].str, sub_str);
                nr_token++;
                break;
            }
          default: {
              tokens[nr_token].type = rules[i].token_type;
              strcpy(tokens[nr_token].str, sub_str);
              nr_token++;
          }
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

bool check_parentheses(int p, int q) {
    if (!(strcmp(tokens[p].str, "(") == 0 && strcmp(tokens[q].str, ")") == 0)) {
        return false;
    }
    int top = 0;
    int i;
    for (i = p + 1; i <= q - 1; i++) {
        if (strcmp(tokens[i].str, "(") == 0) {
            top++;
        }
        else if (strcmp(tokens[i].str, ")") == 0) {
            if (top > 0) {
                top--;
            }
            else return false;
        }
    }
    return top == 0;
}

int find_main_op(int p, int q) {
    int num = 0;
    int temp1 = -1, temp2 = -1, temp3 = -1;
    for (int i = q; i >= p; i--) {
        if (strcmp(tokens[i].str, ")") == 0) {
            num++;
        }
        else if (strcmp(tokens[i].str, "(") == 0) {
            if (num > 0) {
                num--;
            }
            else return -1;
        }
        else if (strcmp(tokens[i].str, "==") == 0) {
            if (num == 0) {
                return i;
            }
        }
        else if (strcmp(tokens[i].str, "+") == 0 || strcmp(tokens[i].str, "-") == 0) {
            if (num == 0) {
                temp1 = i >= temp1 ? i : temp1;
            }
        }
        else if (tokens[i].type == '*' || strcmp(tokens[i].str, "/") == 0) {
            if (num == 0) {
                temp2 = i >= temp2 ? i : temp2;
            }
        }
        else if (tokens[i].type == TK_DEREF) {
            if (num == 0) {
                temp3 = i >= temp3 ? i : temp3;
            }
        }
    }
    return temp1 == -1 ? temp2 == -1 ? temp3 : temp2 : temp1;
}

uint32_t eval(int p, int q) {
    if (p > q) {
        return 0;
    }
    else if (p == q) {

        /* Single token.
         * For now this token should be a number.
         * Return the value of the number.
         */
        extern int atoi(const char*);
        return atoi(tokens[p].str);
    }
    else if (check_parentheses(p, q) == true) {
        /* The expression is surrounded by a matched pair of parentheses.
         * If that is the case, just throw away the parentheses.
         */
        return eval(p + 1, q - 1);
    }
    else {
        int op = find_main_op(p, q);
        if (op == -1) {
            return -1;
        }
        uint32_t val1 = eval(p, op - 1);
        uint32_t val2 = eval(op + 1, q);

        switch (tokens[op].type) {
            case TK_EQ : return val1 == val2;
            case '+': return val1 + val2;
            case '-': return val1 - val2;
            case '*': return val1 * val2;
            case '/':{
                if (val2 == 0) {
                    return -1;
                }
                return val1 / val2;
            }
            case TK_DEREF: {
                extern uint32_t paddr_read(paddr_t, int);
                uint32_t val = paddr_read(val2,4);
                return val;
            }
            default: {
                return -1;
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
  uint32_t res = eval(0, nr_token - 1);
  if (res != -1) {
      *success = true;
  }
  else {
      *success = false;
  }
  return res;
}
