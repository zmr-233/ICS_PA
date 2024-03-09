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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

//为增强进制转换，使用对应宏进行输入输出
#define HEX_SCWORD MUXDEF(CONFIG_ISA64, "%llx", "%x")
#define HEX_PRWORD MUXDEF(CONFIG_ISA64, "%#llx", "%#x")
#define DEC_SCWORD MUXDEF(CONFIG_ISA64, "%"SCNu64, "%"SCNu32)
#define DEC_PRWORD MUXDEF(CONFIG_ISA64, "%"PRIu64, "%"PRIu32)

#define HEX_SCSWORD MUXDEF(CONFIG_ISA64, "%llx", "%x")
#define HEX_PRSWORD MUXDEF(CONFIG_ISA64, "%#llx", "%#x")
#define DEC_SCSWORD MUXDEF(CONFIG_ISA64, "%"SCNd64, "%"SCNd32)
#define DEC_PRSWORD MUXDEF(CONFIG_ISA64, "%"PRId64, "%"PRId32)

// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";
static int nr = 0;
static int buf_i = 0;

#define CHECK_NR(x) (nr+(x)>=32)
#define CHECK_BUF_I (buf_i>=65500)

#define choose(x) (rand()%(x))
#define gen(x) do { buf[buf_i++]=(x); } while(0)
#define MAX_NUM 0x00FFFFFF

static void gen_num() {
  nr++;
  uint32_t num = rand() % MAX_NUM;
  if (choose(2)) { 
    int len = sprintf(&buf[buf_i], "%#x", num);
    buf_i += len;
  } else { 
    int len = sprintf(&buf[buf_i], "%u", num);
    buf_i += len;
  }
}

static void gen_rand_op() {
    int r = choose(17);
    switch (r){
    case 0: case 1: case 2: case 3: case 10: case 11:
      gen('+');
      break;
    case 4: case 5: case 6: case 7: case 14: case 15:
      gen('-');
      break;
    case 8: case 9: 
      gen('*');
      break;
    case 12: case 13: 
      gen('/');
      break;
    case 16:
      gen('=');gen('=');
      break;
    }
    nr++;
}

static void gen_rand_expr() {
  if(CHECK_BUF_I) return;
  if (CHECK_NR(0)) {gen_num(); return;}// 完成表达式
  
  switch (choose(3)) {
    case 0: 
      gen_num(); 
      break;
    case 1:
      if (CHECK_NR(3)) {gen_num(); return;}// 完成表达式
      gen('('); 
      gen_rand_expr(); 
      gen(')');
      nr+=2; 
      
    break;
    default: 
      if (CHECK_NR(3)) {gen_num(); return;}// 完成表达式
      gen_rand_expr(); 
      gen_rand_op(); 
      gen_rand_expr(); 
      break;
  }
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    nr = 0, buf_i = 0;
    nr = rand() % 18 + 15; // 动态设置token长度的上限
    gen_rand_expr();

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) {i--; continue;} //除0这里返回值不为0，因此可以不用考虑

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
