#pragma once
#include<stdio.h>
//获得文件CRC, 使用前先调用 CRC32InitTab(), 否则会返回 0.
unsigned CRC32CalcFile(FILE *fd);
unsigned CRC32CalcFile(const char *pcszFileName);
//获得CRC, 参数 size 请用 sizeof 计算；使用前先调用 CRC32InitTab(), 否则会返回 0.
unsigned CRC32Calc(const void *data, size_t size, unsigned crc_init_value = 0);
