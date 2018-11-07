#include"CRC32.h"

#define Poly 0xEDB88320ul //CRC32标准
static unsigned crc_tab32[256] = { 0 }; //CRC查询表

static class CRC32InitTab
{
public:
	CRC32InitTab();
}functionCRC32InitTab;

//生成CRC查询表
CRC32InitTab::CRC32InitTab()
{
	unsigned crc;
	for (int i = 0; i < 256; i++)
	{
		crc = (unsigned long)i;
		for (int j = 0; j < 8; j++)
		{
			if (crc & 0x00000001L)
				crc = (crc >> 1) ^ Poly;
			else
				crc >>= 1;
		}
		crc_tab32[i] = crc;
	}
}

unsigned CRC32Calc(const void *data, size_t size, unsigned crc_init_value)
{
	unsigned crc = crc_init_value ^ 0xFFFFFFFF;
	int i = 0;

	if (!crc_tab32[1])return 0;
	while (size--)
	{
		crc = (crc >> 8) ^ crc_tab32[(crc & 0xff) ^ *((unsigned char*)data + i)];
		i++;
	}

	return crc ^ 0xFFFFFFFF;
}

unsigned CRC32CalcFile(FILE *fd)
{
	const unsigned size = 16 * 1024;
	unsigned char crcbuf[size];
	size_t rdlen;
	unsigned crc = 0;	//CRC初始值为0

	while ((rdlen = fread(crcbuf, sizeof(unsigned char), size, fd)) > 0)
		crc = CRC32Calc(crcbuf, rdlen, crc);

	return crc;
}

unsigned CRC32CalcFile(const char *pcszFileName)
{
	FILE *f;
#ifdef _MSC_VER
	fopen_s(&f, pcszFileName, "rb");
#else
	f = fopen(pcszFileName, "rb");
#endif
	if (!f)return 0;
	unsigned crc = CRC32CalcFile(f);
	fclose(f);
	return crc;
}