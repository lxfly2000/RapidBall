#pragma once

//������Щ��ֵ�����ʵĴ�С��ʹ���ܼ���������������ֲ�����ռ��̫���ڴ�
#define REPINPUT_U8_INPUT_COUNT 2
#define REPINPUT_U16_INPUT_COUNT 0
#define REPINPUT_U32_INPUT_COUNT 1//ͨ����Ҫ����1��32��64λ���������������
#define REPINPUT_U64_INPUT_COUNT 0


#define REPINPUT_UXX_INPUT_COUNT_MAX 256//����ID�����������ָÿ�����͵������������ж���
#define REPINPUT_UXX_INPUT_MAX(type) (1<<(sizeof(type)*8))//�������͵���󾫶�����

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

static_assert(sizeof(u8) == 1, "sizeof(u8)!=1");
static_assert(sizeof(u16) == 2, "sizeof(u16)!=2");
static_assert(sizeof(u32) == 4, "sizeof(u32)!=4");
static_assert(sizeof(u64) == 8, "sizeof(u64)!=8");

#include<vector>

//�����¼��ʽ��
//u32: ֡��
//u8: idInput
//u8: values.size
//u8...: ��ֵ
struct RepInputEntry
{
	u32 tick;//��ʾ��ǰ֡��ʱ�䣬�������ȵı���
	u8 idInput;
	std::vector<u8> values;

	//����¼����ɾ�����С�����ݣ�����д���ļ���
	//arr: ���λ�ã�����Ϊ�գ�Ϊ��ʱֻ���ظ�����ռ�ֽ�����
	//size: ָ�����λ�õĴ�С���ֽ�������
	//�ɹ�ʱ����������ֽ�������С������ʧ��ʱ����0��
	size_t ToPackedData(u8 *arr, size_t size)const;

	//�Ӵ�������ݶ�ȡ��¼��
	//arr: ����λ�ã�����Ϊ�ա�
	//size: ����λ�õĴ�С���ֽ�������
	//�ɹ�ʱ���ض�ȡ���ֽ�������С���������ݲ���������ʧ��ʱ����0��
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
	RepInputStatus status;//0=����Ϊ����״̬��ȡ�ã�1=�ط�ģʽ��2=¼��ģʽ
	RepInputInputStates inputStates, lastInputStates;
	size_t position;
	u32 tickCounter;
public:
	RepInput();
	void Reset();
	//�����ǰ�ļ�¼��������positionΪ0
	void ClearInputRecords();
	void ClearInputStates();
	size_t LoadFromMem(const u8 *mem, size_t size);
	//�ȶ��뵽�ڴ����ٶ�ȡ�ģ���˲��˹��󣬻����LoadFromMem.
	size_t LoadFromFile(const char *filepath);
	size_t SaveToMem(u8 *mem, size_t size);
	//�ȱ��浽�ڴ�����д���ļ��ģ���˲��˹��󣬻����SaveToMem.
	size_t SaveToFile(const char *filepath);
	bool BeginReplay();
	bool EndReplay();
	bool BeginRecord();
	bool EndRecord();
	//���Ӽ�¼�����Զ�ʹposition+1������MovePositionToNext��������¼��״̬��
	void RecordAddEntry(u8 _idInput, const void *_values, size_t size);
	//��������ֵ���������ֵ�����仯�����Ӽ�¼������RecordAddEntry��
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
	//�������£������ſ���0�ޱ仯�����ް��£��ſ����ޱ仯����״̬��
	int GetInputStateChangeTri(u8 _idInput, size_t size);
};
