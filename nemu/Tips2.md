# C语言补充的相关知识

## 1.X-marcro技术

该技术是为了避免大量的switch/枚举占用空间，能够有效地节省显示的空间，同时也是一种在编译时无法获取Index定的有效技术[原链接](https://quuxplusone.github.io/blog/2021/02/01/x-macros/)

### 1.1 简单INCLUDE技术

用于优化每个记录都具有相同的模式，比如怪物定的低数值类型，也可能是网络错误码的集合(都有一个整数值，消息字符串等等)

```c
//比如错误码: in file "errorcodes.h"
X(EPERM,  1, "Operation not permitted")
X(ENOENT, 2, "No such file or directory")
X(ESRCH,  3, "No such process")
X(EINTR,  4, "Interrupted system call")

//也比如怪物数值集合:  in file "monsters.h"
X(dwarf,     'h', 2, ATK_HIT,  0)
X(kobold,    'k', 2, ATK_HIT,  IMM_POISON)
X(elf,       '@', 3, ATK_HIT,  0)
X(centipede, 'c', 3, ATK_BITE, 0)
X(orc,       'o', 4, ATK_HIT,  IMM_POISON)
```

1. 想要枚举怪物的类型:

   ```c
    enum Monster {
    #define X(name,b,c,d,e) MON_##name,
    #include "monsters.h"
    #undef X
    };
    Monster example = MON_centipede;
   ```

2. 想要构建switch基本块并返回类型

   ```c
        bool is_immune_to_poison(Monster m) {
        switch (m) {
    #define X(name,b,c,d,imm) case MON_##name: return (imm == IMM_POISON);
    #include "monsters.h"
    #undef X
        }
    }
    ```

3. 没有遍历整个集合来获取个数长度，而是直线代码

    ```c
    int number_of_monster_types() {
        return 0
    #define X(a,b,c,d,e) +1
    #include "monsters.h"
    #undef X
        ;
    }
    ```

### 1.2优化X-macro技术

由于这种写法会大量混用include，因此不具有「Idempotence」即为「幂等性」，因此如下变体可以把include写在顶部，但是可能会以损害调试信息的质量为代价:

1. 加上"FOR_"前缀，并定义在列表中; 将宏的名称命名为"DO"

   ```c
   #define FOR_LIST_OF_VARIABLES(DO) \
    DO(id1, name1) \
    DO(id2, name2) \
    DO(id3, name3) \
    ```

2. 使用可变宏参数

    ```c
    void print_variables(void){
    #define PRINT_NAME_AND_VALUE(id, name, ...) printf(#name " = %d\n", name);
    FOR_LIST_OF_VARIABLES( PRINT_NAME_AND_VALUE ) //打印名字的列表 
    

    }
    ```

    ```cpp
    #define DEFINE_ENUMERATION(id, name, ...) name = id, //设置枚举成员的值为id
    enum my_id_list_type {
        FOR_LIST_OF_VARIABLES( DEFINE_ENUMERATION ) //或声明一个枚举
    }
    ```

---

## 2.数组相关

```c
#define SEXT(x, len) (\
{ \
struct { \
    int64_t n : len; \
} __x = { .n = x }; \
(uint64_t)__x.n; \
}) //匿名
```

1. 指定结构体初始化的语法`{ .n=x}`
2. 位域--`int64_t n : len`

### 2.1位域

```c
struct bs{
　　int a:8;
　　int b:2;
　　int c:6;
}data; 

//说明data为bs变量，共占两个字节。其中位域a占8位，位域b占2位，位域c占6位
```

有些信息在存储时，并不需要占用一个完整的字节， 而只需占几个或一个二进制位，所谓“位域”是把一个字节中的二进位划分为几个不同的区域， 并说明每个区域的位数。每个域有一个域名，允许在程序中按域名进行操作。 这样就可以把几个不同的对象用一个字节的二进制位域来表示，但是需要注意:

1. 一个位域必须存储在同一个字节中，不能跨两个字节
2. 由于位域不允许跨两个字节，因此**位域的长度不能大于一个字节的长度**
3. 位域可以无位域名: 这时它只用来作填充或调整位置。无名的位域是不能使用的

    ```c
    struct k{
        int a:1
        int :2 /*无位域名，该2位不能使用*/
        int b:3
        int c:2
    }; 
    ```

4. 结构体长度一定是max(内部最长数据元素, 处理器位数)，意味着上式在32位上是4字节，64位上是8字节

---

## 3.getopt & get_long_opt处理命令行参数

### 3.1 getopt() 短参数

```c
//最简单使用用例 https://www.gnu.org/software/libc/manual/html_node/Example-of-Getopt.html
#include<stdio.h> #include<getopt.h> #include<ctype.h>
int main(int argc, char* argv[]){
    opterr = 0; //A.用于限制getopt手动报错
    char c;
    char* cvalue;
    //第一个循环: 处理getopt参数
    while((c = getopt(argc,argv,"abc:")) != -1){
        switch(c){
            case 'a': break; case 'b': break;
            case 'c': cvalue= optarg; //B.optarg用于从带有参数的地方获取参数
                      break;
            case '?': //C.对于未识别的参数，getopt返回？
                if(optopt=='c') STDERR("ERRROR: -%c requires an argument!",optopt);
                else if (isprint(optopt)) STDERR("ERROR: Unkown -%c",optopt);
                else STDERR("ERROR: Unkown Character!");
                return 1;
            default: abort();
        }
    }
    //第二个循环:处理未知参数
    for (index = optind; index < argc; index++) //D.optind用于下一个要扫描元素索引
        STDERR("Warning: Non-option argument %s\n", argv[index]);
  return 0;
}
```

- 通常下，getopt被用在循环中，当返回-1代表没有更多参数，循环终止;
- 另外switch用于分发处理返回值
- 还要再使用一个循环来处理未处理的参数

```c
#include<getopt.h>
int getopt (int argc, char *argv, char *shortopts)
//自带有三个变量--详情见头文件注释即可
extern char *optarg //B.optarg用于从带有参数的地方获取参数
extern int optind //D.optind用于下一个要扫描元素索引
extern int opterr //A.设置0用于限制getopt报错
extern int optopt //C.设置为未被识别的选项字符--对于未识别的参数，getopt返回'?',同时optopt存储该选项字符
```

### 3.2 getopt_long()处理长参数

```c
static int parse_args(int argc, char *argv[]) {
  const struct option table[] = {
    {"batch"    , no_argument      , NULL, 'b'},
    {"log"      , required_argument, NULL, 'l'},
    {"diff"     , required_argument, NULL, 'd'},
    {"port"     , required_argument, NULL, 'p'},
    {"help"     , no_argument      , NULL, 'h'},
    {0          , 0                , NULL,  0 },
  };
  int o;
  while ( (o = getopt_long(argc, argv, "-bhl:d:p:", table, NULL)) != -1) {
    switch (o) {
      case 'b': sdb_set_batch_mode(); break;
      case 'p': sscanf(optarg, "%d", &difftest_port); break;
      case 'l': log_file = optarg; break;
      case 'd': diff_so_file = optarg; break;
      case 1: img_file = optarg; return 0;
      default:
        printf("Usage: %s [OPTION...] IMAGE [args]\n\n", argv[0]);
        printf("\t-b,--batch              run with batch mode\n");
        printf("\t-l,--log=FILE           output log to FILE\n");
        printf("\t-d,--diff=REF_SO        run DiffTest with reference REF_SO\n");
        printf("\t-p,--port=PORT          run DiffTest with port PORT\n");
        printf("\n");
        exit(0);
    }
  }
  return 0;
}
```

```c
//使用举例如下:
#include<getopt.h>
static int global_flag; //全局参数
static char* target_file; //目标处理对象
int main(int argc, char* argv[]){
    int c, option_index = 0; //显式提供数组下标传入 
    opterr=0;
    while((c = getopt_long(argc,argv,"abc:d:f:",long_options, &option_index)) != -1){
        static struct option long_options[] ={
          //设置了flag, getopt_long不会有返回值:
          {"verbose", no_argument,       &verbose_flag, 1},
          {"brief",   no_argument,       &verbose_flag, 0},
          //flag=0，会满足case '<char>': 的情况
          {"add",     no_argument,       0, 'a'},
          {"create",  required_argument, 0, 'c'},
          {"file",    required_argument, 0, 'f'},
          //会满足case 0:的情况--既没有设置flag，也没有设置val
          {"debug",   no_argument,       0,   0},
          {0, 0, 0, 0}
        };
        switch(c){
            case 0: //长选项既没有设置非零flag，也没有在
                if(long_options[option_index].flag != 0) break;
                STDOUT("Option --%s:", long_options[option_index].name);
                if(optarg) STDOUT("%s",optarg);
            case 1: //特殊情况: 处理非选项参数的情况
                target_file = optarg; //./test -b --log logfile.txt imagefile.img
            case 'a': case 'b': case 'c':
            case '?': /*---参照getopt----*/ 
        }
    }
    if(global_flag) STDOUT("Global_flag is YES");
    //处理未知参数...
    while (optind < argc)  STDERR("Not args: %s /", argv[optind++]);
}
```

- 该函数使用一个结构体`const struct option`来控制选项
  
    结构体参数为struct option `{const char *name,int has_arg,int *flag,int val};` 且**必须要以{0, 0, 0, 0}零向量**作为结尾

    1. 其中has_arg有三个枚举，分别是: `no_argument, required_argument, optional_argument`,用于控制是否有参数

    2. 而flag & val则规则较为特殊: (a)flag=0,则val是getopt返回值——字符缩写 (b)若flag=&(int)num地址,则val是即将要存储进num的值 (c)若flag/val均没有设置,则返回`case 0:`

- 除了前面提到的自带有三个变量，新增一个Index索引: `*indedxptr`用来标识在数组里的下标，比如: `longopts[*indexptr].name`

- 还有一个`getopt_long_only`函数，用于以单个`-`来标注长参数(默认是`--`长参数，`-`短参数), 解析方式为: 比如'./test -foo'，会先以长参数'foo'匹配，未找到再匹配单个匹配短参数'f','o','o'

    ```c
    //函数原型如下
    int getopt_long (int argc, char *const *argv, const char *shortopts, const struct option *longopts, int *indexptr)

    int getopt_long_only (int argc, char *const *argv, const char *shortopts, const struct option *longopts, int *indexptr)
    ```

- **特殊情况**: "case 0:"意味着在long_options中,既没有指定flag也没有val,此时依赖于`.name`去解析; "case 1:"意味着`*shortopts`至少指定了诸如`-abc:`的`-`, 说明是**处理非选项参数的情况**，比如考虑如下场景:`./test -b --log logfile.txt imagefile.img` 其中的`iamgefile.img`就对应于case 1

---
