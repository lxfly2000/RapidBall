#pragma once
#include<stdio.h>
//����ļ�CRC, ʹ��ǰ�ȵ��� CRC32InitTab(), ����᷵�� 0.
unsigned CRC32CalcFile(FILE *fd);
unsigned CRC32CalcFile(const char *pcszFileName);
//���CRC, ���� size ���� sizeof ���㣻ʹ��ǰ�ȵ��� CRC32InitTab(), ����᷵�� 0.
unsigned CRC32Calc(const void *data, size_t size, unsigned crc_init_value = 0);
