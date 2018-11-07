#pragma once

//调整这些数值到合适的大小，使功能既能满足你的需求又不至于占用太多内存
#define REPINPUT_U8_INPUT_COUNT 2
#define REPINPUT_U16_INPUT_COUNT 0
#define REPINPUT_U32_INPUT_COUNT 1//通常需要至少1个32或64位数保存随机数种子
#define REPINPUT_U64_INPUT_COUNT 0


#define REPINPUT_UXX_INPUT_COUNT_MAX 256//输入ID的最大数量，指每种类型的输入最多可以有多少
#define REPINPUT_UXX_INPUT_MAX(type) (1<<(sizeof(type)*8))//各自类型的最大精度数量

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

static_assert(sizeof(u8) == 1, "sizeof(u8)!=1");
static_assert(sizeof(u16) == 2, "sizeof(u16)!=2");
static_assert(sizeof(u32) == 4, "sizeof(u32)!=4");
static_assert(sizeof(u64) == 8, "sizeof(u64)!=8");

#include<vector>

//打包记录格式：
//u32: 帧号
//u8: idInput
//u8: values.size
//u8...: 数值
struct RepInputEntry
{
	u32 tick;//表示当前帧，时间，计数器等的变量
	u8 idInput;
	std::vector<u8> values;

	//将记录打包成尽可能小的数据，用于写入文件。
	//arr: 输出位置，可以为空，为空时只返回该项所占字节数。
	//size: 指定输出位置的大小（字节数）。
	//成功时返回输出的字节数，大小不够或失败时返回0。
	size_t ToPackedData(u8 *arr, size_t size)const;

	//从打包的数据读取记录。
	//arr: 输入位置，不能为空。
	//size: 输入位置的大小（字节数）。
	//成功时返回读取的字节数，大小不够（数据不完整）或失败时返回0。
	size_t FromPackedData(const u8 *arr,size_t size);

	void Clear();

	u8 ValueU8()const;
	u16 ValueU16()const;
	u32 ValueU32()const;
	u64 ValueU64()const;
};

struct RepInputInputStates
{
#if REPINPUT_U64_INPUT_COUNT>0
	u64 u64Inputs[REPINPUT_U64_INPUT_COUNT];
#endif
#if REPINPUT_U32_INPUT_COUNT>0
	u32 u32Inputs[REPINPUT_U32_INPUT_COUNT];
#endif
#if REPINPUT_U16_INPUT_COUNT>0
	u16 u16Inputs[REPINPUT_U16_INPUT_COUNT];
#endif
#if REPINPUT_U8_INPUT_COUNT>0
	u8 u8Inputs[REPINPUT_U8_INPUT_COUNT];
#endif
};

class RepInput
{
public:
	enum RepInputStatus
	{
		REPINPUT_STATUS_NONE,
		REPINPUT_STATUS_REPLAY,
		REPINPUT_STATUS_RECORD
	};
private:
	std::vector<RepInputEntry> inputRecords;
	RepInputStatus status;//0=仅作为键盘状态读取用，1=回放模式，2=录像模式
	RepInputInputStates inputStates, lastInputStates;
	size_t position;
	u32 tickCounter;
public:
	RepInput();
	void Reset();
	//清除当前的记录，并且置position为0
	void ClearInputRecords();
	void ClearInputStates();
	size_t LoadFromMem(const u8 *mem, size_t size);
	//先读入到内存中再读取的，因此不宜过大，会调用LoadFromMem.
	size_t LoadFromFile(const char *filepath);
	size_t SaveToMem(u8 *mem, size_t size);
	//先保存到内存中再写入文件的，因此不宜过大，会调用SaveToMem.
	size_t SaveToFile(const char *filepath);
	bool BeginReplay();
	bool EndReplay();
	bool BeginRecord();
	bool EndRecord();
	//增加记录，会自动使position+1（调用MovePositionToNext），仅限录制状态。
	void RecordAddEntry(u8 _idInput, const void *_values, size_t size);
	//更新输入值，如果输入值发生变化会增加记录（调用RecordAddEntry）
	void UpdateInputState(u8 _idInput, const void *_pvalues, size_t size);
	void UpdateInputStateU8(u8 _idInput, u8 _value);
	void UpdateInputStateU16(u8 _idInput, u16 _value);
	void UpdateInputStateU32(u8 _idInput, u32 _value);
	void UpdateInputStateU64(u8 _idInput, u64 _value);
	void ReplayInputState();
	void MovePositionToBegin();
	void MovePositionToEnd();
	void MovePositionToNext();
	const RepInputEntry * ReplayGetCurrentEntry()const;
	void TickCounterAdd();
	const RepInputInputStates &GetInputStates();
	//正数按下，负数放开，0无变化（仅限按下，放开，无变化三种状态）
	int GetInputStateChangeTri(u8 _idInput, size_t size);
};
