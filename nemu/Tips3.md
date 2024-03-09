REGEX(3) Linux 程序员手册 REGEX(3)

名称
       regcomp, regexec, regerror, regfree - POSIX 正则表达式函数

摘要
       #include <regex.h>

       int regcomp(regex_t *preg, const char *regex, int cflags);

       int regexec(const regex_t *preg, const char *string, size_t nmatch,
                   regmatch_t pmatch[], int eflags);

       size_t regerror(int errcode, const regex_t *preg, char *errbuf,
                       size_t errbuf_size);

       void regfree(regex_t *preg);

描述
   POSIX 正则表达式编译
       regcomp() 用于将正则表达式编译成适合后续 regexec() 搜索的形式。

       regcomp() 需要提供 preg，一个指向模式缓冲区存储区的指针；regex，一个指向空终止字符串的指针；以及 cflags，用于确定编译类型的标志。

       所有正则表达式搜索都必须通过已编译的模式缓冲区进行，因此 regexec() 必须总是使用 regcomp() 初始化的模式缓冲区地址。

       cflags 是以下一个或多个的按位或组合：

       REG_EXTENDED
              使用 POSIX 扩展正则表达式语法来解释 regex。如果未设置，将使用 POSIX 基础正则表达式语法。

       REG_ICASE
              不区分大小写。使用此模式缓冲区进行后续的 regexec() 搜索将不会区分大小写。

       REG_NOSUB
              不报告匹配的位置。如果使用此标志编译的模式缓冲区，则 regexec() 的 nmatch 和 pmatch 参数将被忽略。

       REG_NEWLINE
              匹配任意字符操作符不匹配换行符。

              不包含换行符的不匹配列表 ([^...]) 不匹配换行符。

              行开始操作符 (^) 紧接在换行符后的空字符串匹配，无论 regexec() 的执行标志 eflags 是否包含 REG_NOTBOL。

              行结束操作符 ($) 紧接在换行符前的空字符串匹配，无论 eflags 是否包含 REG_NOTEOL。

   POSIX 正则表达式匹配
       regexec() 用于将空终止字符串与预编译的模式缓冲区 preg 进行匹配。nmatch 和 pmatch 用于提供任何匹配的位置信息。eflags 是以下一个或多个标志的按位或组合：

       REG_NOTBOL
              行开始匹配操作符总是失败（但参见上面的编译标志 REG_NEWLINE）。当字符串的不同部分传递给 regexec() 并且字符串的开始不应被解释为行的开始时，可以使用此标志。

       REG_NOTEOL
              行结束匹配操作符总是失败（但参见上面的编译标志 REG_NEWLINE）。

       REG_STARTEND
              在输入字符串上使用 pmatch[0]，从字节 pmatch[0].rm_so 开始，结束于字节 pmatch[0].rm_eo 之前。这允许匹配嵌入的 NUL 字节，并避免在大字符串上使用 strlen(3)。它不在输入上使用 nmatch，并且不改变 REG_NOTBOL 或 REG_NEWLINE 处理。此标志是 BSD 扩展，在 POSIX 中不存在。

   字节偏移量
       除非为模式缓冲区的编译设置了 REG_NOSUB，否则可以获得匹配地址信息。pmatch 必须配置有至少 nmatch 个元素。regexec() 会用子字符串匹配地址填充它们。存储在第 i 个开括号处开始的子表达式的偏移量存储在 pmatch[i] 中。整个正则表达式的匹配地址存储在 pmatch[0] 中。（注意，要返回 N 个子表达式的偏移量，nmatch 必须至少为 N+1。任何未使用的结构元素将包含值 -1。

regmatch_t 结构是 pmatch 的类型，它在 <regex.h> 中定义。

           typedef struct {
               regoff_t rm_so;
               regoff_t rm_eo;
           } regmatch_t;

       每个 rm_so 元素，如果不是 -1，则表示字符串中下一个最大子字符串匹配的起始偏移量。相对的 rm_eo 元素表示匹配的结束偏移量，即匹配文本后的第一个字符的偏移量。

   POSIX 错误报告
       regerror() 用于将 regcomp() 和 regexec() 可能返回的错误代码转换为错误消息字符串。

       regerror() 接收错误代码 errcode、模式缓冲区 preg、指向字符字符串缓冲区 errbuf 的指针 errbuf，以及字符串缓冲区大小 errbuf_size。它返回包含空终止错误消息字符串所需的 errbuf 大小。如果 errbuf 和 errbuf_size 都不为零，errbuf 将填充为错误消息的前 errbuf_size - 1 个字符和一个终止的空字节 ('\0')。

   POSIX 模式缓冲区释放
       使用已编译的模式缓冲区 preg 提供 regfree() 将释放由编译过程 regcomp() 分配的模式缓冲区的内存。

返回值
       regcomp() 对于成功的编译返回零，对于失败返回错误代码。

       regexec() 对于成功的匹配返回零，对于失败返回 REG_NOMATCH。

错误
       regcomp() 可能返回以下错误：

       REG_BADBR
              无效使用反向引用操作符。

       REG_BADPAT
              无效使用模式操作符，如组或列表。

       REG_BADRPT
              无效使用重复操作符，例如将 '*' 用作第一个字符。

       REG_EBRACE
              未匹配的大括号间隔操作符。

       REG_EBRACK
              未匹配的方括号列表操作符。

       REG_ECOLLATE
              无效的排序元素。

       REG_ECTYPE
              未知的字符类名。

       REG_EEND
              非特定错误。这不是 POSIX.2 定义的。

       REG_EESCAPE
              尾随反斜杠。

       REG_EPAREN
              未匹配的括号组操作符。

       REG_ERANGE
              范围操作符的无效使用；例如，范围的结束点出现在起始点之前。

       REG_ESIZE
              编译的正则表达式需要比 64 kB 大的模式缓冲区。这不是 POSIX.2 定义的。

       REG_ESPACE
              正则表达式运行时内存不足。

       REG_ESUBREG
              对子表达式的无效反向引用。

属性
       有关本节中使用的术语的解释，请参见 attributes(7)。

       ┌─────────────────────┬───────────────┬────────────────┐
       │接口                │ 属性          │ 值             │
       ├─────────────────────┼───────────────┼────────────────┤
       │regcomp(), regexec() │ 线程安全      │ MT-Safe 区域   │
       ├─────────────────────┼───────────────┼────────────────┤
       │regerror()           │ 线程安全      │ MT-Safe 环境   │
       ├─────────────────────┼───────────────┼────────────────┤
       │regfree()            │ 线程安全      │ MT-Safe        │
       └─────────────────────┴───────────────┴────────────────┘

符合性
       POSIX.1-2001，POSIX.1-2008。

示例
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

另请参阅
       grep(1)，regex(7)

       glibc 手册章节，正则表达式

编者
       本页是 Linux man-pages 项目版本 5.10 的一部分。有关该项目的描述、报告错误信息以及本页的最新版本，请访问 https://www.kernel.org/doc/man-pages/。

GNU                                                  2020-08-13                                            REGEX(3)