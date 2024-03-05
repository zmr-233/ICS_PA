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
//这里面是客户程序:
#include <isa.h>
#include <memory/paddr.h>

// this is not consistent with uint8_t
// but it is ok since we do not access the array directly
/*
一些用于调试的指令:
NEMU选择了ebreak指令来充当nemu_trap.为了表示客户程序是否成功结束
 当客户程序执行了这条指令之后, NEMU将会根据这个结束状态参数来设置NEMU的结束状态, 
 并根据不同的状态输出不同的结束信息, 主要包括

HIT GOOD TRAP - 客户程序正确地结束执行
HIT BAD TRAP - 客户程序错误地结束执行
ABORT - 客户程序意外终止, 并未结束执行
*/
//客户程序如下:
static const uint32_t img [] = {
  0x00000297,  // auipc t0,0
  0x00028823,  // sb  zero,16(t0)
  0x0102c503,  // lbu a0,16(t0)
  0x00100073,  // ebreak (used as nemu_trap)
  0xdeadbeef,  // some data
};

static void restart() {
  /* Set the initial program counter. */
  cpu.pc = RESET_VECTOR;

  /* The zero register is always 0. */
  cpu.gpr[0] = 0;
}

void init_isa() {
  /*
  
将CPU将要访问的内存地址映射到pmem中的相应偏移位置, 这是通过nemu/src/memory/paddr.c中的guest_to_host()函数实现的.
mips32和riscv32的物理地址均从0x80000000开始,CONFIG_MBASE将会被定义成0x80000000
例如如果mips32的CPU打算访问内存地址0x80000000, 我们会让它最终访问pmem[0]
  */
  /* Load built-in image. */
  memcpy(guest_to_host(RESET_VECTOR), img, sizeof(img)); //实现了内存映射
/*
为了检查你的实现是否正确, 我们在init_isa()中还调用了reg_test()函数
(在nemu/src/isa/x86/reg.c中定义). 具体会在下文的必做题中进行介绍.
*/
  /* Initialize this virtual computer system. */
  restart();
}
