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
#include <utils.h>
#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
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

//2.继续运行(1) c
static int cmd_c(char *args) {
  cpu_exec(-1); //进入指令执行的主循环
  return 0;
}

//3.退出(1)
extern NEMUState nemu_state;
static int cmd_q(char *args) {
  nemu_state.state = NEMU_QUIT;
  return -1;
}

#ifdef NEXTUSE
//4.单步执行
/*si [N] 让程序单步执行N条指令后暂停执行, 当N没有给出时, 缺省为1*/
static int cmd_si(char* args){

  return 0;
}
#endif

//5.打印程序状态
/* info w 打印监视点信息/info r打印寄存器状态*/
extern void isa_reg_display(void); //打印所有寄存器API
static int cmd_info(char* args){
  isa_reg_display();
  return 0;
}

//6.扫描内存(2)
/*x N EXPR 求出表达式EXPR的值, 将结果作为起始内存地址
x 10 $esp 以十六进制形式输出连续的N个4字节*/
static int cmd_x(char* args){
  printf("|%s|",args);
  return 0;
}

#ifdef NEXTUSE

//7.表达式求值
/*p EXPR 求出表达式EXPR的值, EXPR支持的运算请见调试中的表达式求值小节
https://nju-projectn.github.io/ics-pa-gitbook/ics2024/1.6.html
p $eax + 1*/
static int cmd_p(char* args){

  return 0;
}

//8.设置监视点
/*w EXPR 当表达式EXPR的值发生变化时,暂停程序执行
w *0x2000*/
static int cmd_w(char* args){

  return 0;
}

//9.删除监视点 d N 删除序号为N的监视点
static int cmd_d(char* args){

  return 0;
}
#endif
/*
函数，是通过cmd_table[i](..)调用的
*/

static int cmd_help(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q }, 
  //TEMP:
  { "i", "Info Register", cmd_info }, 
  { "x", "Scan Memory", cmd_x },

  /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)

//1.帮助(1)---打印命令的帮助信息
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



void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    //if (*cmd == -1 ){ return ;}

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    //匹配命令项
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

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
