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
#include <errno.h>

enum {
  TK_NOTYPE = 256, 
  TK_NEG, //负号
  TK_DEREF, //解引用
  TK_AND, //&&
  TK_OR, //||
  TK_EQ, TK_NE, //== !=
  TK_GT, TK_LT, TK_GE, TK_LE, // > < >= <=
  TK_HEX, TK_DEC, TK_REG, TK_VAR // 16进制数，10进制数，寄存器，变量

  /*解引用放在循环进行

  */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */
  {" +", TK_NOTYPE},    // spaces
  {"<=", TK_LE},        // less or equal
  {">=", TK_GE},        // greater or equal
  {"==", TK_EQ},        // equal
  {"!=", TK_NE},         // not equal
  {"&&", TK_AND},        // and
  {"\\|\\|", TK_OR},         // or
  {"<", TK_LT},         // less than
  {">", TK_GT},         // greater than
  {"\\+", '+'},         // plus
  {"-", '-'},           // sub
  {"\\*", '*'},         // multiply
  {"/", '/'},           // divide
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

static Token tokens[48] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

#define CHECK_NR_TOKEN if(nr_token >= 47) { printf("Too many tokens\n"); return false; }

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

        IFDEF(make_tokenLog, Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start));

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
            tokens[nr_token].str[substr_len] = '\0'; //！！！！！！！！！！该错误极为隐秘
            IFDEF(make_tokenLog, Log("stdnpcy tokens[nr_token].str :%s",tokens[nr_token].str));
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
static void print_tokens(){
  Log("Print tokens: ");
  for(int i=0; i<nr_token; i++)
    if(tokens[i].type < 256) Log(" >>%c", tokens[i].type);
    else Log(" >>%d: %s", tokens[i].type, tokens[i].str);
  Log("---------------");
}

static bool check_parentheses(int p, int q,bool* success){
  if(tokens[p].type != '(' || tokens[q].type != ')') return false;
  int cnt = 0;
  for(int i=p; i<=q; i++){
    if(tokens[i].type == '(') cnt++;
    else if(tokens[i].type == ')') cnt--;
    if(cnt == 0 && i != q) return false;
  }
  return true;
}
//将字符串转换为数字
word_t str2word(char *s, bool *success, int base){
  errno = 0; char* endptr; word_t tmp;
  /*if(s[0] == '0' && s[1] == 'x') 
    tmp = MUXDEF(CONFIG_ISA64, strtoull(s, &endptr, 16), strtoul(s, &endptr, 16));
  else 
    tmp = MUXDEF(CONFIG_ISA64, strtoull(s, &endptr, 10), strtoul(s, &endptr, 10));
    */
  tmp = MUXDEF(CONFIG_ISA64, strtoull(s, &endptr, base), strtoul(s, &endptr, base));
  if(errno != 0) {*success = false; Log("Error: errno = %d",errno); errno = 0; 
    /*是否需要*/return 0;}
  if(endptr==s) {*success = false; Log("Error: No digits were found!");return 0;}
  if(*endptr != '\0') Log("Warning: Further characters after number: %s",endptr);
  return tmp;
}
sword_t str2sword(char *s, bool *success, int base){
  errno = 0; char* endptr; sword_t tmp;
  tmp = MUXDEF(CONFIG_ISA64, strtoll(s, &endptr, base), strtol(s, &endptr, base));
  if(errno != 0) {*success = false; Log("Error: errno = %d",errno); errno = 0; 
    /*是否需要*/return 0;}
  if(endptr==s) {*success = false; Log("Error: No digits were found!");return 0;}
  if(*endptr != '\0') Log("Warning: Further characters after number: %s",endptr);
  return tmp;
}

int64_t str2int64(char *s, bool *success, int base){
  errno = 0; char* endptr; int64_t tmp;
  tmp = strtoll(s, &endptr, base);
  if(errno != 0) {*success = false; Log("Error: errno = %d",errno); errno = 0; 
    /*是否需要*/return 0;}
  if(endptr==s) {*success = false; Log("Error: No digits were found!");return 0;}
  if(*endptr != '\0') Log("Warning: Further characters after number: %s",endptr);
  IFDEF(str2int64Log, Log("str2int64() tmp :%"PRId64,tmp));
  return tmp;
}
#define getMainOpLog 1
static int getMainOp(int p, int q, bool* success){
  IFDEF(getMainOpLog, Log("GetMainOp() p: %d, q: %d, success: %d",p,q,success?1:0));
  int op = -1, op_prio = __INT_MAX__, cur_prio = 0;
  for(int i=p; i<=q; i++){
    //1.跳过括号:
    if(tokens[i].type == '(') {
      IFDEF(getMainOpLog, Log("Skip left bracket at %d",i));
      int cnt = 1; i++;//跳过当前括号
      while(cnt && i<=q){
        if(tokens[i].type == '(') cnt++; 
        else if(tokens[i].type == ')') cnt--;
        i++;
      } 
      //i++; //修正位置--不需要
      //continue; //避免在处理括号后用cur_prio影响到op_prio--使用了两个if，不需要了
    }
    //2.处理token:
    if(tokens[i].type == ')') {
      Log("Error: Unmatched right bracket at %d",i);
      *success = false;
      return -1;
    }
    else if(tokens[i].type == TK_AND) cur_prio = 1;
    else if(tokens[i].type == TK_OR) cur_prio = 2;
    else if(tokens[i].type == TK_EQ || tokens[i].type == TK_NE) cur_prio = 3;
    else if(tokens[i].type == TK_GT 
    || tokens[i].type == TK_LT
    || tokens[i].type == TK_GE
    || tokens[i].type == TK_LE) cur_prio = 4;
    else if(tokens[i].type == '+' || tokens[i].type == '-') cur_prio = 5;
    else if(tokens[i].type == '*' || tokens[i].type == '/') cur_prio = 6;
    else if(tokens[i].type == TK_NEG || tokens[i].type == TK_DEREF) cur_prio = 7;
    else if(tokens[i].type == TK_DEC 
    || tokens[i].type == TK_HEX 
    || tokens[i].type == TK_REG 
    || tokens[i].type == TK_VAR) 
      continue;
    else {
      Log("Error: Invalid token type: %d",tokens[i].type);
      *success = false;
      return -1;
    }
    //根据优先级+结合性选择主运算符
    //if(cur_prio < op_prio || (cur_prio == op_prio && op==-1)) { 
    if(cur_prio <= op_prio || op==-1) {   
      IFDEF(getMainOpLog, Log("if(cur_prio <= op_prio) cur_prio=%d, op_prio=%d",cur_prio,op_prio));
      op = i; op_prio = cur_prio;
    }
  }
  if(op < p || op > q)
    Assert(op < p || op > q || *success == false,"getMainOpLog: Bad expression : main op = %d",op);
  return op;
}

#define evalLog 1
int64_t eval(int p, int q, bool* success) {
  IFDEF(evalLog, Log("Eval() p: %d, q: %d",p,q));
  if (p > q || !*success || p<0 || q>nr_token-1) {
    Log("Error: if (p > q || !*success || p<0 || q>nr_token-1)");
    *success |= false;
    return 0;
  }
  else if (p == q) {
    IFDEF(evalLog, Log("else if (p == q)"));
    switch(tokens[p].type){
      case TK_HEX: return str2int64(tokens[p].str, success, 16);
      case TK_DEC: return str2int64(tokens[p].str, success, 10);
      case TK_REG: return (int64_t)isa_reg_str2val(tokens[p].str, success); //注意:需要分别处理$0和$a0
      case TK_VAR: *success=false; TODO(); return 0; //TODO:变量的值
      default: 
        Log("Error: Invalid token type: %d",tokens[p].type); *success=false; return 0;
    }
  }
  else if (check_parentheses(p, q, success) == true) {
    IFDEF(evalLog, Log("else if (check_parentheses(p, q, success) == true)"));
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     */
    return eval(p + 1, q - 1, success);
  }
  else {
    Log("else:");
    int op = getMainOp(p, q, success);
    Log("op = %d",op);
    if(!*success) {Log("Error: Bad expression : main op"); return 0;}
    //运算符分为单目+双目 | 短路+非短路
    switch(tokens[op].type){
    case TK_NEG:
      int64_t neg = -eval(op + 1, q, success);
      if(!*success) {Log("Error: Bad expression : TK_NEG-neg"); return 0;}
      IFDEF(evalLog, Log("Info : return TK_NEG :%"PRId64,neg));
      return neg;
    case TK_DEREF:
      Assert(TK_REG,"TODO() TK_DEREF");
    default: //默认双目运算符
      int64_t val1 = eval(p, op - 1, success);
      //处理短路
      if(!*success) {Log("Error: Bad expression : val1"); return 0;}
      switch(tokens[op].type){
        case TK_AND: if(val1 == 0) return 0;
        case TK_OR: if(val1 != 0) return 1;
      }
      int64_t val2 = eval(op + 1, q, success);
      if(!*success) {Log("Error: Bad expression : val2"); return 0;}
      switch (tokens[op].type) {
        case '+': 
          IFDEF(evalLog, Log("Info :val1+val2 :%"PRId64,val1 + val2));
          return val1 + val2;
        case '-':
          IFDEF(evalLog, Log("Info :val1-val2 :%"PRId64,val1 - val2));
          return val1 - val2;
        case '*': 
          IFDEF(evalLog, Log("Info :val1*val2 :%"PRId64,val1 * val2));
          return val1 * val2;
        case '/': 
          if(val2!=0){
            IFDEF(evalLog, Log("Info :val1/val2 :%"PRId64,val1 / val2));
            return val1 / val2;
          }
          Log("Error: Devide zero!");
          *success = false; return 0;
        //&&
        case TK_AND: 
          return val1 && val2;
        //||
        case TK_OR: 
          return val1 || val2;
        //== !=
        case TK_EQ: 
          return val1 == val2;
        case TK_NE: 
          return val1 != val2;
        //> < >= <=
        case TK_GT: 
          return val1 > val2;
        case TK_LT: 
          return val1 < val2;
        case TK_GE: 
          return val1 >= val2;
        case TK_LE: 
          return val1 <= val2;
        default:
          *success = false;
          Log("Error: Unkown main op : %c",tokens[op].type);
          return 0;
        /* TODO(): 实现短路的诶功能
        */
      }
    }
  }
}

//#define exprLog 1
int64_t expr(char *e, bool *success) {
  IFDEF(exprLog, Log("Expr() e: %s",e));
  if (!make_token(e)) { *success = false;return 0;}
  print_tokens();
  //用于处理负号+解引用
  for(int i=0; i<nr_token; i++){
    int ptype = tokens[i-1].type;
    if(tokens[i].type == '-' && (i==0 || (ptype < 256 && ptype != ')'))){
      tokens[i].type = TK_NEG; IFDEF(exprLog, Log("tokens[%d].type = TK_NEG",i));
    }
    if(tokens[i].type == '*' && (i==0 || (ptype < 256 && ptype != ')'))){
      tokens[i].type = TK_DEREF; IFDEF(exprLog, Log("tokens[%d].type = TK_DEREF",i));
    }
  }
  int64_t tmp = eval(0, nr_token-1, success);
  if(!*success) {Log("Error: Bad expression : eval"); return 0;}
  IFDEF(exprLog, Log("info : expr() :%"PRId64,tmp));
  return tmp;
}
