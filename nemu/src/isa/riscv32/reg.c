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
#include <macro.h>
#include "local-include/reg.h"

/*
$0：这是一个硬编码的零寄存器，任何写入此寄存器的值都会被忽略，读取永远返回0。

ra：返回地址寄存器，通常用于存储函数调用返回后的地址。

sp：堆栈指针寄存器，用于跟踪程序堆栈的顶部。

gp：全局指针寄存器，通常用于存储全局变量的地址。

tp：线程指针寄存器，用于多线程编程。

t0-t6：临时寄存器，可以被子程序自由使用，不需要保存和恢复其值。

s0-s11：保存寄存器，如果子程序需要使用这些寄存器，它必须保存原始值并在返回前恢复。

a0-a7：参数寄存器，用于在函数调用中传递参数和返回结果。


*/

const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

//新增: 描述数组
const char *desp[] = {
    "Zero register         ", // $0: Hard-wired to 0
    "Return address        ", // ra: Return address for jumps
    "Stack pointer         ", // sp: Pointer to the top of the stack
    "Global pointer        ", // gp: Points to the global data
    "Thread pointer        ", // tp: Thread-local storage pointer
    "Temporary/scratch reg ", // t0: Temporary register 0
    "Temporary/scratch reg ", // t1: Temporary register 1
    "Temporary/scratch reg ", // t2: Temporary register 2
    "Saved register 0      ", // s0: Saved register 0/frame pointer
    "Saved register 1      ", // s1: Saved register 1
    "Function argument 0   ", // a0: Function argument 0/return value 0
    "Function argument 1   ", // a1: Function argument 1/return value 1
    "Function argument 2   ", // a2: Function argument 2
    "Function argument 3   ", // a3: Function argument 3
    "Function argument 4   ", // a4: Function argument 4
    "Function argument 5   ", // a5: Function argument 5
    "Function argument 6   ", // a6: Function argument 6
    "Function argument 7   ", // a7: Function argument 7
    "Saved register 2      ", // s2: Saved register 2
    "Saved register 3      ", // s3: Saved register 3
    "Saved register 4      ", // s4: Saved register 4
    "Saved register 5      ", // s5: Saved register 5
    "Saved register 6      ", // s6: Saved register 6
    "Saved register 7      ", // s7: Saved register 7
    "Saved register 8      ", // s8: Saved register 8
    "Saved register 9      ", // s9: Saved register 9
    "Saved register 10     ", // s10: Saved register 10
    "Saved register 11     ", // s11: Saved register 11
    "Temporary/scratch reg ", // t3: Temporary register 3
    "Temporary/scratch reg ", // t4: Temporary register 4
    "Temporary/scratch reg ", // t5: Temporary register 5
    "Temporary/scratch reg "  // t6: Temporary register 6
};


//extern CPU_state cpu;


/* 注意:#include "local-include/reg.h"已经定义了部分函数:
#define gpr(idx) (cpu.gpr[check_reg_idx(idx)])

int check_reg_idx(int idx)

const char* reg_name(int idx)

const char* reg_desp(int idx)
*/

/**
 * CONFIG_RVE 是一个编译时的配置选项:
 * 用于指定是否使用 RISC-V 的嵌入式扩展（Embedded Extension，即 E 扩展）
 * RISC-V 的 E 扩展启用, 只要求有 16 个寄存器，而不是标准的 32 个寄存器
*/

//1.打印所有寄存器的值
void isa_reg_display() {
  printf("Registers:\n");
  for(int idx=0; idx < MUXDEF(CONFIG_RVE, 16, 32);idx++){
    printf("%-4s:  0x%08x  %-23s\n", reg_name(idx), gpr(idx), reg_desp(idx));
  }
  printf("\n");
}

//2.用于取出寄存器的值
word_t isa_reg_str2val(const char *s, bool *success) {
  *success = false;
  if(strcmp(s,reg_name(0)) == 0){*success = true; return 0;}
  for(int idx=1; idx < MUXDEF(CONFIG_RVE, 16, 32);idx++){
    if(strcmp(s+1, reg_name(idx)) == 0){
      *success = true;
      return gpr(idx);
    }
  }
  return 0;
}
