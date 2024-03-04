# 学习到的各种技巧

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

## Make & Config

### |技巧:使用konfig关联是否编译

使用如下的`$(CONFIG_TARGET_AM)`，设置TARGET_AM将会设置该选项的值是y/n，其中只有DIRS-BLACKLIST-y才会编译，间接实现了选择编译的效果

```Makefile
DIRS-BLACKLIST-$(CONFIG_TARGET_AM) += src/monitor/sdb
```

已经在`autoconf.h`中定义了`CONFIG_XXX`的宏，但是需要在C中来判断这些宏是否定义，为此在`nemu/include/macro.h`中定义了专门用来对宏进行测试的宏:

