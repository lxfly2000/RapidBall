#pragma once
#define LCG_RAND_MAX lcg_get_rand_max()
#ifdef __cplusplus
extern"C"{
#endif
//设置随机数参数（线性同余法）
//参考：http://www.cnblogs.com/xkfz007/archive/2012/03/27/2420154.html
void lcg_set_param(int a, int c, int rand_max);
//获取最大随机数
int lcg_get_rand_max();
//设置随机数种子
void lcg_srand(unsigned int seed);
//生成随机数（数值范围为0～LCG_RAND_MAX(0x7FFF)）
int lcg_rand();
#ifdef __cplusplus
}
#endif
