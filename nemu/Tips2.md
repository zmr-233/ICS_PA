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

## 4.readline库---bash的同款命令行读取

### 4.1基础使用

该readline库被广泛用于mail,ftp,sh等命令行参数的解析:因为readline.h中的一些定义使用了stdio库，所以文件<stdio.h>应该包含在readline.h之前[链接地址](https://tiswww.case.edu/php/chet/readline/readline.html#Programming-with-GNU-Readline)

- 可以放入一行的提示词prompt
- 返回值是内部使用malloc()申请，意味着必须要在外部调用free释放
- 返回的只有文本,换行符被去除了
- 如果接收到EOF,就会返回(char*)NULL
- 使用`add_history`能够加入历史，但是**尽量不要把NULL加入历史**
- TAB默认是补充文件名称的

```c
char *readline (const char *prompt); //主要读取函数
add_history (line); //添加到可读写历史中
int rl_bind_key (int key, rl_command_func_t *function); //绑定按键的函数
/*key是要绑定的字符，function是按下键时要调用的函数的地址
rl_bind_key ('\t', rl_insert);禁用默认标签行为
该代码应该在程序开始时执行一次,可以编写一个名为 的函数initialize_readline()来执行此初始化和其他所需的初始化
*/
```

```c
#include <readline/readline.h>
#include <readline/history.h>
static char *line_read = (char *)NULL; //用一个static去进行处理
char * rl_gets (){//EOF为NULL,其余返回地址
  if (line_read){ 
      free (line_read); //如果被使用，请进行释放
      line_read = (char *)NULL;
    }
  line_read = readline ("(bash) $ ");//提示符
  if (line_read && *line_read)
    add_history (line_read); //存入历史中
  return (line_read);
}
```

### 4.2自定义补全

见相关博客[略](https://www.cnblogs.com/hazir/p/instruction_to_readline.html)

---

## 5.str字符串函数

|如何获取字符串相关函数: 使用`man 3 str<tab><tab>`就可以查看库函数中的str字符串处理函数

以下是一个使用Markdown格式的表，其中列出了上述函数及其简短的中文介绍：

| 函数名 | 描述 | 函数名 | 描述 |
| ------ | ---- | ------ | ---- |
| `strcasecmp` | 忽略大小写比较字符串 | `strcasestr` | 忽略大小写查找子串 |
| `strcat` | 连接两个字符串 | `strchr` | 查找字符在字符串中首次出现的位置 |
| `strchrnul` | 查找字符在字符串中首次出现的位置，若未找到返回字符串尾后null | `strcmp` | 比较两个字符串 |
| `strcoll` | 根据程序当前的locale比较两个字符串 | `strcpy` | 复制字符串 |
| `strcspn` | 计算两字符串中首个共同字符前的字符数 | `strdup` | 复制字符串，使用malloc分配内存 |
| `strdupa` | 复制字符串，使用alloca分配内存 | `strerror` | 返回错误码对应的错误信息 |
| `strerrordesc_np` | 返回错误码对应的错误描述 | `strerror_l` | 返回错误码对应的本地化错误信息 |
| `strerrorname_np` | 返回错误码对应的宏名称 | `strfmon` | 格式化货币字符串 |
| `strfmon_l` | 格式化货币字符串（本地化） | `strfromd` | 将double类型数字转换为字符串 |
| `strfromf` | 将float类型数字转换为字符串 | `strfroml` | 将long double类型数字转换为字符串 |
| `strfry` | 随机排列字符串 | `strftime` | 格式化时间 |
| `string` | 处理字符串数组的函数 | `string_to_av_perm` | 将字符串转换为访问向量权限 |
| `string_to_security_class` | 将字符串转换为安全类 | `strncasecmp` | 忽略大小写比较两个字符串的前n个字符 |
| `strncat` | 连接两个字符串的前n个字符 | `strncmp` | 比较两个字符串的前n个字符 |
| `strncpy` | 复制指定长度的字符串 | `strndup` | 复制指定长度的字符串，使用malloc分配内存 |
| `strndupa` | 复制指定长度的字符串，使用alloca分配内存 | `strnlen` | 获取字符串的长度，最大为指定值 |
| `strpbrk` | 在字符串中查找任何一个指定的字符 | `strptime` | 解析时间字符串 |
| `strrchr` | 查找字符在字符串中最后一次出现的位置 | `strsep` | 分割字符串 |
| `strsignal` | 返回信号码对应的信号描述 | `strspn` | 计算字符串开头连续包含指定字符集内字符的最大长度 |
| `strstr` | 查找子串在字符串中首次出现的位置 | `strtod` | 将字符串转换为double类型 |
| `strtof` | 将字符串转换为float类型 | `strtoimax` | 将字符串转换为最大整数类型 |
| `strtok` | 分割字符串 | `strtok_r` | 线程安全的分割字符串 |
| `strtol` | 将字符串转换为long整型 | `strtold` | 将字符串转换为long double类型 |
| `strtoll` | 将字符串转换为long long整型 | `strtoq` | 将字符串转换为quad_t类型 |
| `strtoul` | 将字符串转换为无符号long整型 | `strtoull` | 将字符串转换为无符号long long整型 |
| `strtoumax` | 将字符串转换为最大无符号整数类型 | `strtouq` | 将字符串转换为无符号quad_t类型 |
| `strverscmp` | 比较两个版本排序字符串 | `strxfrm` | 根据当前locale转换字符串以进行比较 |

请注意，一些函数可能如`strdupa`和`strndupa`是GNU C库的扩展，不是标准C的一部分

### 1.strtok分割字符串(分割参数)

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(int argc, char *argv[]) {
    char *str1, *str2, *token, *subtoken;
    char *saveptr1, *saveptr2;
    int j = 1;
    for (str1 = argv[1]; ; j++, str1 = NULL) {
        token = strtok_r(str1, argv[2], &saveptr1);
        if (token == NULL) break;
        printf("%d: %s\n", j, token);

        for (str2 = token; ; str2 = NULL) {
            subtoken = strtok_r(str2, argv[3], &saveptr2);
            if (subtoken == NULL) break;
            printf(" --> %s\n", subtoken);
        }
    }
}/*$ ./a.out 'a/bbb///cc;xxx:yyy:' ':;' '/'
1: a/bbb///cc   2: xxx        3: yyy
        --> a      --> xxx       --> yyy
        --> bbb
        --> cc */
```

strtok()能够根据delimit(定界符)参数，把该定界符替换为'\0'，并返回对应位置的指针。其原理是在内部有一个buf，能够存储传入的字符串+上次的指针，因此调用应该使用：

- 连续调用strtok才能获得一连串token，但是第二次调用开始，必须传入NULL，否则会覆盖原字符串
- 由于采用静态buf，因此有strtok_r, 使用外部指针saveptr来保存，可以多线程使用

```c
char* strtok(char* str, const char*delim);
    token1 = strtok(str,",;"); //第一次调用: 必须是非NULL
    token2 = strtok(NULL,",;"); // 第二次调用: 必须是NULL
char* strtok_r(char* str,const cahr *delim, char **seveptr); //线程安全-可重入
```

### 2. strtol/strtoll/strtoq字符串转数字函数

这几个函数用于将字符串转换为长整数，其中可以使用base来指定基数类型。

- 开头可以有任意数量的空字符(使用isspace(3)来判定), 在第一个非有效数字处停下, 并把位置通过endptr来传递
- 如果没有任何数字，strtol()返回0，同时endptr=nptr;如果`*endptr=='\0'`表明解析力整个字符串，并没有任何非法字符
- 对于underflow/overflow会相应地返回`LONG_MIN/LONG_MAX`，意味着要**判断错误必须要通过判断`errno`来进行**，而对于strtoll来说会返回`LLONG_MIN/LLONG_MAX`

对于errno:

- `EINVAL`:给定的基数不支持(有实现对于空转换也会设置这个错误码)
- `ERANGE`:结果超出值域

```c
#include <stdlib.h>
#include <errno.h>
long strtol(const char *nptr, char **endptr, int base);
long long strtoll(const char *nptr, char **endptr, int base);
//使用举例:
errno = 0;//显式设置为0
val = strtol(str, &endptr, base);
if(errno!=0) {perror("strtol"); exit(EXIT_FAILURE);}
if(endptr==str){STDERR("No digits were found");}
if(*endptr!='\0') STDERR("Warning too many arguments"); //不应该作为错误
```

---

## 6.C其他函数

### 6.1 sscanf()从字符串(流)读取格式化数据

相比起scanf从流中读取，sscanf则是从字符串中根据格式化字符串进行读取，原型如下:

```c
int sscanf(const char* str, const char* format,...);//使用举例:
int day, year;
char weekday[20], month[20], dtm[100];
strcpy( dtm, "Saturday March 25 1989" ); //从这个字符串里读取数据
sscanf( dtm, "%s %s %d  %d", weekday, month, &day, &year );
printf("%s %d, %d = %s\n", month, day, year, weekday );

```

### 6.2 打印uint64_t和uint32_t类型的值

uint64_t和uint32_t是在C99标准中引入的，位于stdint.h或cstdint头文件中，推荐使用<inttypes.h> 包含一些宏:

- Print用: `PRIu64`和`PRIu32`
- Scanf用: `SCNu64`和`SCNu32`

```c
#include <inttypes.h> // 包含PRIu32和PRIu64宏
addr = sscanf(str2, MUXDEF(CONFIG_ISA64, PRIu64, PRIu32), &addr);

//针对32/64位+ HEX/DEC的平台移植输出需要，定义了一些宏:
#define HEX_SCWORD MUXDEF(CONFIG_ISA64, "%llx", "%x")
#define HEX_PRWORD MUXDEF(CONFIG_ISA64, "%#llx", "%#x")
#define DEC_SCWORD MUXDEF(CONFIG_ISA64, "%"SCNu64, "%"SCNu32)
#define DEC_PRWORD MUXDEF(CONFIG_ISA64, "%"PRIu64, "%"PRIu32)
sscanf(str2, HEX_SCWORD, &addr);  //比较方便
Log("x %d : "HEX_PRWORD"\n",N, addr);
```

---

## 7.POSIX<regex.h>正则表达式

```c
#include <regex.h>

int regcomp(regex_t *preg, const char *regex, int cflags);
/*
 preg, a pointer to a pattern buffer storage area;
 regex, a pointer to  the  null-terminated string
 cflags, flags used to determine the type of compilation.
*/
int regexec(const regex_t *preg, const char *string, size_t nmatch,
            regmatch_t pmatch[], int eflags);

size_t regerror(int errcode, const regex_t *preg, char *errbuf,
                size_t errbuf_size);

void regfree(regex_t *preg);

```

```c
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <regex.h>

#define ARRAY_SIZE(arr) (sizeof((arr)) / sizeof((arr)[0]))

static const char *const str =
        "1) John Driverhacker;\n2) John Doe;\n3) John Foo;\n";
static const char *const re = "John.*o";

int main(void)
{
    static const char *s = str;
    regex_t     regex;
    regmatch_t  pmatch[1];
    regoff_t    off, len;

    if (regcomp(&regex, re, REG_NEWLINE))
        exit(EXIT_FAILURE);

    printf("String = \"%s\"\n", str);
    printf("Matches:\n");

    for (int i = 0; ; i++) {
        if (regexec(&regex, s, ARRAY_SIZE(pmatch), pmatch, 0))
            break;

        off = pmatch[0].rm_so + (s - str);
        len = pmatch[0].rm_eo - pmatch[0].rm_so;
        printf("#%d:\n", i);
        printf("offset = %jd; length = %jd\n", (intmax_t) off,
                (intmax_t) len);
        printf("substring = \"%.*s\"\n", len, s + pmatch[0].rm_so);

        s += pmatch[0].rm_eo;
    }

    exit(EXIT_SUCCESS);
}
```
