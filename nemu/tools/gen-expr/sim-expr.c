#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>

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
"#include <stdint.h>\n"
"#include <inttypes.h>\n"
"\n"
"int main() { \n"
"  int64_t result = %s; \n"
"  printf(\"%%ld\", result); \n"
"  return 0; \n"
"}";

int main() {
  int i;
  while(true) {
    printf("-----------");
    printf("Input expr :");
    scanf("%[^\n]", buf);
    getchar();
    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) {
        i--;
        puts("Exp gcc error!");
        abort();
    } //除0这里返回值不为0，因此可以不用考虑

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int64_t result;
    ret = fscanf(fp, "%"PRId64, &result);
    pclose(fp);

    printf("HEX :%#"PRIx64", DEC :%"PRId64"\n", result, result);
  }
  return 0;
}
