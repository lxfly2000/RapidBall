#pragma once
#define LCG_RAND_MAX lcg_get_rand_max()
#ifdef __cplusplus
extern"C"{
#endif
//�������������������ͬ�෨��
//�ο���http://www.cnblogs.com/xkfz007/archive/2012/03/27/2420154.html
void lcg_set_param(int a, int c, int rand_max);
//��ȡ��������
int lcg_get_rand_max();
//�������������
void lcg_srand(unsigned int seed);
//�������������ֵ��ΧΪ0��LCG_RAND_MAX(0x7FFF)��
int lcg_rand();
#ifdef __cplusplus
}
#endif
