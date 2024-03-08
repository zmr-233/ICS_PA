/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ,
  TK_HEX, TK_DEC, TK_REG, TK_VAR
  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ},        // equal
  {"-", '-'},           // sub
  {"/", '/'},           // divide
  {"\\*", '*'},         // multiply
  {"\\(", '('},         // left bracket
  {"\\)", ')'},         // right bracket
  {"0x[0-9a-fA-F]+", TK_HEX}, // hex
  {"[0-9]+", TK_DEC},  // decimal
  {"\\$[a-zA-Z0-9]+", TK_REG}, // register
  {"[a-zA-Z_][a-zA-Z0-9_]*", TK_VAR}, // identifier/variable


};

#define NR_REGEX ARRLEN(rules)

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

#define CHECK_NR_TOKEN if(nr_token >= 32) { printf("Too many tokens\n"); return false; }

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      //如下是通过e+position指定string的起始位置，让regexec一旦匹配，pmatch.rm_so必定为0
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
          case TK_NOTYPE: break;
          case TK_HEX:
          case TK_DEC:
          case TK_REG:
          case TK_VAR:
          CHECK_NR_TOKEN
            tokens[nr_token].type = rules[i].token_type;
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            nr_token++;
            break;
          default: //运算符只需要token_type
          CHECK_NR_TOKEN
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

/*

char *e：这是一个指向字符数组（字符串）的指针，它包含了需要被求值的表达式。
bool *success：这是一个指向布尔值的指针，用来指示表达式是否成功求值。如果表达式成功求值，这个指针指向的值将被设置为 true；如果求值过程中发生错误（例如，表达式语法不正确），则设置为 false。

1.make_token(e)：这个函数看起来是用来对输入的表达式字符串 e 进行词法分析（tokenization），将其分解成一系列的令牌（tokens）

expr 函数的目标是实现一个表达式求值器，但是目前的代码只是一个框
*/
void print_tokens(){
  for(int i=0; i<nr_token; i++)
    if(tokens[i].type < 256) printf(" >>%c\n", tokens[i].type);
    else printf(" >>%d: %s\n", tokens[i].type, tokens[i].str);
}

word_t expr(char *e, bool *success) {
  Log("%s",e);
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  print_tokens();
  /* TODO: Insert codes to evaluate the expression. */
  TODO();

  return 0;
}
