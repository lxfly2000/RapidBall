#include"lcg_random.h"

//各种编译器采用的随机数参数：
//http://www.cnblogs.com/xkfz007/archive/2012/03/27/2420154.html

//此处采用MSVC标准
static int lcg_mod = 0x8000;
static int lcg_a = 214013;
static int lcg_c = 2531011;

static unsigned int x = 1;

void lcg_set_param(int a, int c, int rand_max)
{
	lcg_a = a;
	lcg_c = c;
	lcg_mod = rand_max + 1;
}

int lcg_get_rand_max()
{
	return lcg_mod - 1;
}

void lcg_srand(unsigned int seed)
{
	x = seed;
}

int lcg_rand()
{
	x = lcg_a*x + lcg_c;
	return (unsigned int)(x >> 16) % lcg_mod;
}
