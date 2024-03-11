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

#ifndef __SDB_H__
#define __SDB_H__

#include <common.h>

int64_t expr(char *e, int *success);

word_t vaddr_read(vaddr_t addr, int len);
/*
罪魁祸首: int64_t但是是以word_t返回的!!!
*/

#define FOR_NAME_ENUM(DO) \
    DO("int",                TK_INT,              1) \
    DO("signed int",         TK_INT,              1) \
    DO("unsigned int",       TK_UINT,             2) \
    DO("unsigned",           TK_UINT,             2) \
                                                     \
    DO("short",              TK_SHORT,            3) \
    DO("short int",          TK_SHORT,            3) \
    DO("unsigned short",     TK_USHORT,           4) \
                                                     \
    DO("long",               TK_LONG,             5) \
    DO("unsigned long",      TK_ULONG,            6) \
                                                     \
    DO("long long",          TK_LLONG,            7) \
    DO("unsigned long long", TK_ULLONG,           8) \
                                                     \
    DO("char",               TK_CHAR,             9) \
    DO("unsigned char",      TK_UCHAR,            10) \
                                                     \
    DO("float",              TK_FLOAT,            11) \
    DO("double",             TK_DOUBLE,           12) \
    DO("long double",        TK_LDOUBLE,          13) \
                                                     \
    DO("_Bool",              TK_BOOL,             14) \
    DO("bool",               TK_BOOL,             14) \
                                                     \
    DO("int32_t",            TK_INT32,            15) \
    DO("uint32_t",           TK_UINT32,           16) \
    DO("int32",              TK_INT32,            15)  \
    DO("uint32",             TK_UINT32,           16) \
                                                        \
    DO("int64_t",            TK_INT64,            17)  \
    DO("uint64_t",           TK_UINT64,           18) \
    DO("int64",              TK_INT64,            17)  \
    DO("uint64",             TK_UINT64,           18) \
                                                        \
    DO("int*",               TK_INT_P,            19)  \
    DO("signed int*",        TK_INT_P,            19)  \
    DO("unsigned int*",      TK_UINT_P,           20) \
    DO("unsigned*",          TK_UINT_P,           20) \
                                                        \
    DO("short*",             TK_SHORT_P,          21) \
    DO("short int*",         TK_SHORT_P,          21) \
    DO("unsigned short*",    TK_USHORT_P,         22) \
                                                        \
    DO("long*",              TK_LONG_P,           23) \
    DO("unsigned long*",     TK_ULONG_P,          24) \
                                                        \
    DO("long long*",         TK_LLONG_P,          25) \
    DO("unsigned long long*",TK_ULLONG_P,         26) \
                                                        \
    DO("char*",              TK_CHAR_P,           27) \
    DO("unsigned char*",     TK_UCHAR_P,          28) \
                                                        \
    DO("float*",             TK_FLOAT_P,          29) \
    DO("double*",            TK_DOUBLE_P,         30) \
    DO("long double*",       TK_LDOUBLE_P,        31) \
                                                        \
    DO("_Bool*",             TK_BOOL_P,           32) \
    DO("bool*",              TK_BOOL_P,           32) \
                                                        \
    DO("int32_t*",           TK_INT32_P,          33) \
    DO("uint32_t*",          TK_UINT32_P,         34) \
    DO("int32*",             TK_INT32_P,          33) \
    DO("uint32*",            TK_UINT32_P,         34) \
                                                        \
    DO("int64_t*",           TK_INT64_P,          35) \
    DO("uint64_t*",          TK_UINT64_P,         36) \
    DO("int64*",             TK_INT64_P,          35) \
    DO("uint64*",            TK_UINT64_P,         36) \
    DO("void*",              TK_VOID_P,           37) \
                                                    

#define POINTER_BYTE MUXDEF(CONFIG_ISA64, 8, 4)

#define FOR_ENUM_TYPE(DO) \
    DO("int",                1, TK_INT,              int,                "%d",    sizeof(int)) \
    DO("unsigned int",       2, TK_UINT,             unsigned int,       "%u",    sizeof(unsigned int)) \
    DO("short",              3, TK_SHORT,            short,              "%hd",   sizeof(short)) \
    DO("unsigned short",     4, TK_USHORT,           unsigned short,     "%hu",   sizeof(unsigned short)) \
    DO("long",               5, TK_LONG,             long,               "%ld",   sizeof(long)) \
    DO("unsigned long",      6, TK_ULONG,            unsigned long,      "%lu",   sizeof(unsigned long)) \
    DO("long long",          7, TK_LLONG,            long long,          "%lld",  sizeof(long long)) \
    DO("unsigned long long", 8, TK_ULLONG,           unsigned long long, "%llu",  sizeof(unsigned long long)) \
    DO("char",               9, TK_CHAR,             char,               "%c",    sizeof(char)) \
    DO("unsigned char",     10, TK_UCHAR,            unsigned char,      "%c",    sizeof(unsigned char)) \
    DO("float",             11, TK_FLOAT,            float,              "%f",    sizeof(float)) \
    DO("double",            12, TK_DOUBLE,           double,             "%lf",   sizeof(double)) \
    DO("long double",       13, TK_LDOUBLE,          long double,        "%Lf",   sizeof(long double)) \
    DO("_Bool",             14, TK_BOOL,             _Bool,              "%d",    sizeof(_Bool)) \
    DO("int32_t",           15, TK_INT32,            int32_t,            "%" PRId32, sizeof(int32_t)) \
    DO("uint32_t",          16, TK_UINT32,           uint32_t,           "%" PRIu32, sizeof(uint32_t)) \
    DO("int64_t",           17, TK_INT64,            int64_t,            "%" PRId64, sizeof(int64_t)) \
    DO("uint64_t",          18, TK_UINT64,           uint64_t,           "%" PRIu64, sizeof(uint64_t)) \
    \
    DO("int*",              19, TK_INT_P,            int*,               "%p", POINTER_BYTE) \
    DO("unsigned int*",     20, TK_UINT_P,           unsigned int*,      "%p", POINTER_BYTE) \
    DO("short*",            21, TK_SHORT_P,          short*,             "%p", POINTER_BYTE) \
    DO("unsigned short*",   22, TK_USHORT_P,         unsigned short*,    "%p", POINTER_BYTE) \
    DO("long*",             23, TK_LONG_P,           long*,              "%p", POINTER_BYTE) \
    DO("unsigned long*",    24, TK_ULONG_P,          unsigned long*,     "%p", POINTER_BYTE) \
    DO("long long*",        25, TK_LLONG_P,          long long*,         "%p", POINTER_BYTE) \
    DO("unsigned long long*",26,TK_ULLONG_P,         unsigned long long*,"%p", POINTER_BYTE) \
    DO("char*",             27, TK_CHAR_P,           char*,              "%p", POINTER_BYTE) \
    DO("unsigned char*",    28, TK_UCHAR_P,          unsigned char*,     "%p", POINTER_BYTE) \
    DO("float*",            29, TK_FLOAT_P,          float*,             "%p", POINTER_BYTE) \
    DO("double*",           30, TK_DOUBLE_P,         double*,            "%p", POINTER_BYTE) \
    DO("long double*",      31, TK_LDOUBLE_P,        long double*,       "%p", POINTER_BYTE) \
    DO("_Bool*",            32, TK_BOOL_P,           _Bool*,             "%p", POINTER_BYTE) \
    DO("int32_t*",          33, TK_INT32_P,          int32_t*,           "%p", POINTER_BYTE) \
    DO("uint32_t*",         34, TK_UINT32_P,         uint32_t*,          "%p", POINTER_BYTE) \
    DO("int64_t*",          35, TK_INT64_P,          int64_t*,           "%p", POINTER_BYTE) \
    DO("uint64_t*",         36, TK_UINT64_P,         uint64_t*,          "%p", POINTER_BYTE) \
    DO("void*",             37, TK_VOID_P,           void*,              "%p", POINTER_BYTE)
           



#define EXPAND_AS_ENUM(name, id, enumName, typeName,...) enumName=id,
typedef enum TypeEnum {
    FOR_ENUM_TYPE(EXPAND_AS_ENUM)
    TK_UNTYPE
}TypeEnum;

//为增强进制转换，使用对应宏进行输入输出
#define HEX_SCWORD MUXDEF(CONFIG_ISA64, "%"PRIx64, "%"PRIx32)
#define HEX_PRWORD MUXDEF(CONFIG_ISA64, "%#"PRIx64, "%#"PRIx32)
#define DEC_SCWORD MUXDEF(CONFIG_ISA64, "%"SCNu64, "%"SCNu32)
#define DEC_PRWORD MUXDEF(CONFIG_ISA64, "%"PRIu64, "%"PRIu32)

#define HEX_SCSWORD MUXDEF(CONFIG_ISA64, "%"PRIx64, "%"PRIx32)
#define HEX_PRSWORD MUXDEF(CONFIG_ISA64, "%#"PRIx64, "%#"PRIx32)
#define DEC_SCSWORD MUXDEF(CONFIG_ISA64, "%"SCNd64, "%"SCNd32)
#define DEC_PRSWORD MUXDEF(CONFIG_ISA64, "%"PRId64, "%"PRId32)


//检查是否是合法内存地址
#define CHECK_ADDR(x) (x < 0 || x > MUXDEF(CONFIG_ISA64, UINT64_MAX, UINT32_MAX))
#endif
