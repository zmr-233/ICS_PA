# 学习到的各种技巧

## Linux指令

```bash
sudo dmesg #查看linux启动日志
```

---

## GIT技巧

### |技巧:.gitignore

```bash
*.* #忽略所有包含点的文件(目录)
*   #忽略所有没有扩展名文件
!*/ #不忽略目录
!Makefile 
!*.mk
!*.[cSh] #不忽略扩展名为.c .S .h文件
!*.cc    #不忽略.cc的C++文件
!.gitignore
!README.md
!Kconfig
include/config
include/generated

!*.md
```

---

## Make & Config

### |技巧:使用konfig关联是否编译

使用如下的`$(CONFIG_TARGET_AM)`，设置TARGET_AM将会设置该选项的值是y/n，其中只有DIRS-BLACKLIST-y才会编译，间接实现了选择编译的效果

```Makefile
DIRS-BLACKLIST-$(CONFIG_TARGET_AM) += src/monitor/sdb
```

### |技巧: 在C中判断macro宏定义是否已经定义

已经在`autoconf.h`中定义了`CONFIG_XXX`的宏，但是需要在C中来判断这些宏是否定义，为此在`nemu/include/macro.h`中定义了专门用来对宏进行测试的宏:

```c
//1.编译器前面加上#可以直接作为字符串""
#define str(x) #x //使用举例：printf("%s",str(HELLO_WORLD));
#define STRLEN(CONST_STR) (sizeof(CONST_STR) - 1)// strlen() for string constant
#define ARRLEN(arr) (int)(sizeof(arr) / sizeof(arr[0])) // calculate the length of an array

//2.连接字符串 macro concatenation--依赖于编译器使用 HELLO ## WORLD 能够输出HELLOWORLD
#define concat_temp(x, y) x ## y
#define concat(x, y) concat_temp(x, y)
#define concat3(x, y, z) concat(concat(x, y), z)
#define concat4(x, y, z, w) concat3(concat(x, y), z, w)
#define concat5(x, y, z, v, w) concat4(concat(x, y), z, v, w)

//3.利用是否带有逗号来选择a or b的宏
// See https://stackoverflow.com/questions/26099745/test-if-preprocessor-symbol-is-defined-inside-macro
#define CHOOSE2nd(a, b, ...) b
#define MUX_WITH_COMMA(contain_comma, a, b) CHOOSE2nd(contain_comma a, b)
#define MUX_MACRO_PROPERTY(p, macro, a, b) MUX_WITH_COMMA(concat(p, macro), a, b)
// define placeholders for some property
#define __P_DEF_0  X,
#define __P_DEF_1  X,
#define __P_ONE_1  X,
#define __P_ZERO_0 X,
// define some selection functions based on the properties of BOOLEAN macro
#define MUXDEF(macro, X, Y)  MUX_MACRO_PROPERTY(__P_DEF_, macro, X, Y)
#define MUXNDEF(macro, X, Y) MUX_MACRO_PROPERTY(__P_DEF_, macro, Y, X)
#define MUXONE(macro, X, Y)  MUX_MACRO_PROPERTY(__P_ONE_, macro, X, Y)
#define MUXZERO(macro, X, Y) MUX_MACRO_PROPERTY(__P_ZERO_,macro, X, Y)

/* 这里的“MUX”和“PROPERTY”分别代表“multiplexer”（多路选择器）和“属性”
使用举例: MUXDEF(CONFIG_ITRACE, puts(str(YES)),puts(str(NO)));
这里其实是依赖于CONFIG_ITRACE被定义-> 
    ！！注意：必须要定义为 #define CONFIG_ITRACE 1 为一个值！！
    ————因为解析的宏实际上是依赖于__P_DEF_1 X,的逗号的

    这里的宏设计非常有意思:
        - 只针对DEF: __P_DEF_0 __P_DEF_1 意味着值是0是1均可
        - 只针对值: __P_ONE_1 要求必须是1才会执行

如下都是对这几个函数的包装了:
*/
#define ISDEF(macro) MUXDEF(macro, 1, 0) // test if a boolean macro is defined
#define ISNDEF(macro) MUXNDEF(macro, 1, 0) // test if a boolean macro is undefined
#define ISONE(macro) MUXONE(macro, 1, 0) // test if a boolean macro is defined to 1
#define ISZERO(macro) MUXZERO(macro, 1, 0) // test if a boolean macro is defined to 0

//4.最有用的宏: test if a macro of ANY type is defined
// NOTE1: it ONLY works inside a function, since it calls `strcmp()`
// NOTE2: macros defined to themselves (#define A A) will get wrong results
// NOTE3: 对于带有参数的#define TEST13(x,y,z) (x, y, z)，这里的isdef(TEST13(,,))必须也有逗号；嫌麻烦也可以只使用isdef(TEST)
#define isdef(macro) (strcmp("" #macro, "" str(macro)) != 0)

//5.多路选择器--分支执行部分: simplification for conditional compilation
#define __IGNORE(...)
#define __KEEP(...) __VA_ARGS__ 
// keep the code if a boolean macro is defined
#define IFDEF(macro, ...) MUXDEF(macro, __KEEP, __IGNORE)(__VA_ARGS__)
// keep the code if a boolean macro is undefined
#define IFNDEF(macro, ...) MUXNDEF(macro, __KEEP, __IGNORE)(__VA_ARGS__)
// keep the code if a boolean macro is defined to 1
#define IFONE(macro, ...) MUXONE(macro, __KEEP, __IGNORE)(__VA_ARGS__)
// keep the code if a boolean macro is defined to 0
#define IFZERO(macro, ...) MUXZERO(macro, __KEEP, __IGNORE)(__VA_ARGS__)

//6.一种强大X--macro技术，之后会提及
//functional-programming-like macro (X-macro)
// apply the function `f` to each element in the container `c`
// NOTE1: `c` should be defined as a list like:
//   f(a0) f(a1) f(a2) ...
// NOTE2: each element in the container can be a tuple
#define MAP(c, f) c(f)

//7.位处理相关
#define BITMASK(bits) ((1ull << (bits)) - 1) //创建一个位掩码，其长度为bits,BITMASK(3)会产生一个值0b111
#define BITS(x, hi, lo) (((x) >> (lo)) & BITMASK((hi) - (lo) + 1)) // similar to x[hi:lo] in verilog 这与Verilog硬件描述语言中的位切片操作类似
#define SEXT(x, len) ({ struct { int64_t n : len; } __x = { .n = x }; (uint64_t)__x.n; }) //匿名结构体+位域
/*用途可能是在处理位操作时：
需要确保一个值的符号位在进行某些计算或转换之前被正确地扩展
例如，在将一个较短的有符号整数转换为一个较长的有符号整数时，可能需要保持其符号位不变*/

//8.地址对齐相关
#define ROUNDUP(a, sz)   ((((uintptr_t)a) + (sz) - 1) & ~((sz) - 1)) //ROUNDUP将地址a向上舍入到最接近的sz的倍数
#define ROUNDDOWN(a, sz) ((((uintptr_t)a)) & ~((sz) - 1)) //将地址a向下舍入到最接近的sz的倍数 
#define PG_ALIGN __attribute((aligned(4096)))
/*使用例子: PG_ALIGN char my_array[4096]; 
my_array将会被对齐到一个4096字节的边界:意味着my_array的地址将是4096的倍数
*/

//9.CPU分支预测优化
/*This is not about function definitions. It is about rearranging the machine code in a way that causes a smaller probability for the CPU to fetch instructions that are not going to be executed.
使用方法: if(__builtin_expect(cond,0)) foo(); else bar(); 实际代码会把bar放到前面
*/
#if !defined(likely)
#define likely(cond)   __builtin_expect(cond, 1) //判断结果最可能为真
#define unlikely(cond) __builtin_expect(cond, 0) //判断结果最可能为假
#endif

//10.从寄存器中读取和写入数据 for AM IOE
#define io_read(reg) \
  ({ reg##_T __io_param; \
    ioe_read(reg, &__io_param); \
    __io_param; })

#define io_write(reg, ...) \
  ({ reg##_T __io_param = (reg##_T) { __VA_ARGS__ }; \
    ioe_write(reg, &__io_param); })
```

使用举例:

```c
#include<stdio.h>
#include<macro.h>
//#define __P_DEF_CONFIG_ITRACE X,
//或者:
#define CONFIG_ITRACE 1
int main(){
    IFDEF(CONFIG_ITRACE, puts("Hello world"));

    MUXDEF(CONFIG_ITRACE, puts(str(YES)),puts(str(NO)));
}
```
