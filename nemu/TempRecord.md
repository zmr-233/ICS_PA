# 总体框架

```bash
ics2023
├── abstract-machine   # 抽象计算机
├── am-kernels         # 基于抽象计算机开发的应用程序
├── fceux-am           # 红白机模拟器
├── init.sh            # 初始化脚本
├── Makefile           # 用于工程打包提交
├── nemu               # NEMU
└── README.md
```

## nemu/

```bash
nemu
├── configs                    # 预先提供的一些配置文件
├── include                    # 存放全局使用的头文件
│   ├── common.h               # 公用的头文件
│   ├── config                 # 配置系统生成的头文件, 用于维护配置选项更新的时间戳
│   ├── cpu
│   │   ├── cpu.h
│   │   ├── decode.h           # 译码相关
│   │   ├── difftest.h
│   │   └── ifetch.h           # 取址相关
│   ├── debug.h                # 一些方便调试用的宏
│   ├── device                 # 设备相关
│   ├── difftest-def.h
│   ├── generated
│   │   └── autoconf.h         # 配置系统生成的头文件, 用于根据配置信息定义相关的宏
│   ├── isa.h                  # ISA相关
│   ├── macro.h                # 一些方便的宏定义--定义了专门用来对宏进行测试的宏
│   ├── memory                 # 访问内存相关
│   └── utils.h
├── Kconfig                    # 配置信息管理的规则
├── Makefile                   # Makefile构建脚本
├── README.md
├── resource                   # 一些辅助资源
├── scripts                    # Makefile构建脚本
│   ├── build.mk
│   ├── config.mk
│   ├── git.mk                 # git版本控制相关
│   └── native.mk
├── src                        # 源文件
│   ├── cpu
│   │   └── cpu-exec.c         # 指令执行的主循环
│   ├── device                 # 设备相关
│   ├── engine
│   │   └── interpreter        # 解释器的实现
│   ├── filelist.mk
│   ├── isa                    # ISA相关的实现
│   │   ├── mips32
│   │   ├── riscv32
│   │   ├── riscv64
│   │   └── x86
│   ├── memory                 # 内存访问的实现
│   ├── monitor
│   │   ├── monitor.c          # 将客户程序读入计算机
│   │   └── sdb                # 简易调试器
│   │       ├── expr.c         # 表达式求值的实现
│   │       ├── sdb.c          # 简易调试器的命令处理
│   │       └── watchpoint.c   # 监视点的实现
│   ├── nemu-main.c            # 你知道的...
│   └── utils                  # 一些公共的功能
│       ├── log.c              # 日志文件相关
│       ├── rand.c
│       ├── state.c
│       └── timer.c
└── tools                      # 一些工具
    ├── fixdep                 # 依赖修复, 配合配置系统进行使用
    ├── gen-expr
    ├── kconfig                # 配置系统
    ├── kvm-diff
    ├── qemu-diff
    └── spike-diff
```

### 相关指令

```bash
make nemuconfig # 进行编译配置

```

### 1.Kconfig配置系统

[相关链接](https://nju-projectn.github.io/ics-pa-gitbook/ics2024/1.3.html)
配置系统位于nemu/tools/kconfig，"配置描述文件"的文件名都为Kconfig,目前只用关心配置系统生成如下文件:

```bash
#目前关注如下文件
nemu/include/generated/autoconf.h #阅读C代码时使用--可以被包含到C代码中的宏定义
nemu/include/config/auto.conf #阅读Makefile时使用--可以被包含到Makefile中的变量定义

#不重要的东西：
#nemu/include/config/auto.conf.cmd #被包含到Makefile中的, 和"配置描述文件"相关的依赖规则
#nemu/include/config/ #通过时间戳来维护配置选项变化的目录树-配合另一个工具nemu/tools/fixdep来使用
```

#### 1.1Makefile相关

文件列表: 在nemu/src下的`filelist.mk`, 维护如下四个变量:
    ```Makefile
    SRCS-y #参与编译的源文件的候选集合
    SRCS-BLACKLIST-y #不参与编译的源文件的黑名单集合
    DIRS-y #参与编译的目录集合, 该目录下的所有文件都会被加入到SRCS-y中
    DIRS-BLACKLIST-y #不参与编译目录集合, 该目录下的所有文件都会被加入SRCS-BLACKLIST-y中
    ```
    Makefile会汇总项目中所有定的filelist.mk

##### |技巧:使用konfig关联是否编译

使用如下的`$(CONFIG_TARGET_AM)`，设置TARGET_AM将会设置该选项的值是y/n，其中只有DIRS-BLACKLIST-y才会编译，间接实现了选择编译的效果

```Makefile
DIRS-BLACKLIST-$(CONFIG_TARGET_AM) += src/monitor/sdb
```

#### 1.2编译规则

Makefile的编译规则在`nemu/scripts/build.mk`中定义

| 技巧: 使用`make -nB`能够相当于预览了即将要执行的所有指令

---

### 2.NEMU主程序

1. 内置客户程序位于 `nemu/src/isa/$ISA/init.c`中
2. 内存通过在nemu/src/memory/paddr.c中定义的大数组pmem来模拟的128MB的`pmem`数组(静态段)
   总是使用vaddr_read()和vaddr_write()(在nemu/src/memory/vaddr.c中定义)来访问模拟的内存
   vaddr, paddr分别代表虚拟地址和物理地址
3. 约定：客户程序的位置---读入到固定的内存位置:`nemu/include/memory/paddr.h`定义`RESET_VECTOR`
4. 寄存器结构体定义CPU_state:`nemu/src/isa/$ISA/include/isa-def.h`，并在`nemu/src/cpu/cpu-exec.c`中定义一个全局变量cpu(cpu.pc就是寄存器)
5. TRM的工作方式通过cpu_exec()和exec_once()体现

#### 三个调试宏

- `Log()`是printf()的升级版, 专门用来输出调试信息, 同时还会输出使用Log()所在的源文件, 行号和函数. 当输出的调试信息过多的时候, 可以很方便地定位到代码中的相关位置
- `Assert()`是assert()的升级版, 当测试条件为假时, 在assertion fail之前可以输出一些信息
- `panic()`用于输出信息并结束程序, 相当于无条件的assertion fail
