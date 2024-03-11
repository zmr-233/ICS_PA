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
#include "local-include/reg.h"

/*
$0：这通常代表一个常数寄存器，其值永远为0。

ra："return address"（返回地址）
tp："thread pointer"（线程指针）用于指向线程局部存储。
sp："stack pointer"（栈指针），用于指向当前栈帧的顶部。

a0-a7：这些是用于函数参数和返回值的寄存器-
        在函数调用中，a0-a1通常用于返回值，a0-a7用于前8个整数参数。
t0-t6："temporary"（临时）寄存器，用于存储临时数据
        它们在函数调用时不需要被保存（caller saved）。

s0-s8："saved"（保存的）寄存器，用于存储长期使用的值
        它们在函数调用时需要被保存（callee saved）
  s0/fp同样可以作为"frame pointer"（帧指针）使用，指向当前栈帧的底部。

fp：虽然列出为一个单独的寄存器，但通常与 s0 相同，作为"frame pointer"使用。

t7-t8：这些也是临时寄存器

rs：这个寄存器的名称不是 RISC-V 的标准寄存器命名-没有遵循通用的 RISC-V 寄存器命名约定。

*/

const char *regs[] = {
  "$0", "ra", "tp", "sp", "a0", "a1", "a2", "a3",
  "a4", "a5", "a6", "a7", "t0", "t1", "t2", "t3",
  "t4", "t5", "t6", "t7", "t8", "rs", "fp", "s0",
  "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8"
};


void isa_reg_display() {
}

word_t isa_reg_str2val(const char *s, int *success) {
  return 0;
}
/*
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

别看错了： 这是loongarch32r架构！！！！！！！！！！！！！！！！

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*/
