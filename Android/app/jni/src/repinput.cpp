#include "repinput.h"
#include <fstream>
#include <string>

size_t RepInputEntry::ToPackedData(u8 * arr, size_t size) const
{
	if (arr && sizeof(tick) + sizeof(idInput) + values.size() * sizeof(decltype(values[0])) > size)
		return 0;
	size_t sumsize = 0;
	if (arr)
		memcpy(arr + sumsize, &tick, sizeof(tick));
	sumsize += sizeof(tick);
	if (arr)
		memcpy(arr + sumsize, &idInput, sizeof(idInput));
	sumsize += sizeof(idInput);
	u8 plength = static_cast<u8>(values.size());
	if (arr)
		memcpy(arr + sumsize, &plength, sizeof(plength));
	sumsize += sizeof(plength);
	for (u8 i = 0; i < plength; i++)
	{
		if (arr)
			arr[sumsize] = values[i];
		sumsize += sizeof(values[i]);
	}
	return sumsize;
}

size_t RepInputEntry::FromPackedData(const u8 * arr, size_t size)
{
	if (size < sizeof(tick) + sizeof(idInput) + sizeof(u8))
		return 0;
	size_t sumsize = 0;
	memcpy(&tick, arr + sumsize, sizeof(tick));
	sumsize += sizeof(tick);
	memcpy(&idInput, arr + sumsize, sizeof(idInput));
	sumsize += sizeof(idInput);
	u8 plength = *reinterpret_cast<const u8*>(arr + sumsize);
	sumsize += sizeof(plength);
	for (u8 i = 0; i < plength; i++)
	{
		if (sumsize + sizeof(values[i]) > size)
			return 0;
		values.push_back(arr[sumsize]);
		sumsize += sizeof(arr[sumsize]);
	}
	return sumsize;
}

void RepInputEntry::Clear()
{
	values.clear();
}

u8 RepInputEntry::ValueU8() const
{
	return values[0];
}

u16 RepInputEntry::ValueU16() const
{
	return *reinterpret_cast<const u16*>(values.data());
}

u32 RepInputEntry::ValueU32() const
{
	return *reinterpret_cast<const u32*>(values.data());
}

u64 RepInputEntry::ValueU64() const
{
	return *reinterpret_cast<const u64*>(values.data());
}

RepInput::RepInput()
{
	Reset();
}

void RepInput::Reset()
{
	switch (status)
	{
	case RepInput::REPINPUT_STATUS_REPLAY:EndReplay(); break;
	case RepInput::REPINPUT_STATUS_RECORD:EndRecord(); break;
	default:status = REPINPUT_STATUS_NONE; break;
	}
	ClearInputRecords();
}

void RepInput::ClearInputRecords()
{
	inputRecords.clear();
	MovePositionToBegin();
}

void RepInput::ClearInputStates()
{
#if REPINPUT_U64_INPUT_COUNT>0
	memset(inputStates.u64Inputs, 0, sizeof(inputStates.u64Inputs));
#endif
#if REPINPUT_U32_INPUT_COUNT>0
	memset(inputStates.u32Inputs, 0, sizeof(inputStates.u32Inputs));
#endif
#if REPINPUT_U16_INPUT_COUNT>0
	memset(inputStates.u16Inputs, 0, sizeof(inputStates.u16Inputs));
#endif
#if REPINPUT_U8_INPUT_COUNT>0
	memset(inputStates.u8Inputs, 0, sizeof(inputStates.u8Inputs));
#endif
}

size_t RepInput::LoadFromMem(const u8 * mem, size_t size)
{
	RepInputEntry entry;
	size_t readBytes, totalBytes = 0;
	while (totalBytes<size)
	{
		entry.Clear();
		if (size < totalBytes)
			return 0;
		readBytes = entry.FromPackedData(mem + totalBytes, size - totalBytes);
		if (readBytes == 0)
			return 0;
		inputRecords.push_back(entry);
		totalBytes += readBytes;
	}
	return totalBytes;
}

size_t RepInput::LoadFromFile(const char * filepath)
{
	//https://github.com/lxfly2000/pmdplay/blob/master/pmdplay/PMDPlayer.cpp#L121
	std::fstream f(filepath, std::ios::binary | std::ios::in);
	if (f)
	{
		std::string sf((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
		return LoadFromMem(reinterpret_cast<const u8*>(sf.data()), sf.size());
	}
	return 0;
}

size_t RepInput::SaveToMem(u8 * mem, size_t size)
{
	size_t writtenBytes, totalBytes = 0;
	for (size_t i = 0; i < inputRecords.size(); i++)
	{
		if (mem&&size < totalBytes)
			return 0;
		writtenBytes = inputRecords[i].ToPackedData(mem ? mem + totalBytes : mem, size - totalBytes);
		if (writtenBytes == 0)
			return 0;
		totalBytes += writtenBytes;
	}
	return totalBytes;
}

size_t RepInput::SaveToFile(const char * filepath)
{
	size_t memlength = SaveToMem(nullptr, 0);
	u8 *mem = new u8[memlength];
	if (SaveToMem(mem, memlength))
	{
		std::fstream f(filepath, std::ios::binary | std::ios::out);
		if (f)
		{
			f.write(reinterpret_cast<char*>(mem), memlength);
			return memlength;
		}
	}
	return 0;
}

bool RepInput::BeginReplay()
{
	if (status == REPINPUT_STATUS_NONE)
	{
		status = REPINPUT_STATUS_REPLAY;
		tickCounter = 0;
		MovePositionToBegin();
		return true;
	}
	return false;
}

bool RepInput::EndReplay()
{
	if (status == REPINPUT_STATUS_REPLAY)
	{
		status = REPINPUT_STATUS_NONE;
		return true;
	}
	return false;
}

bool RepInput::BeginRecord()
{
	if (status == REPINPUT_STATUS_NONE)
	{
		status = REPINPUT_STATUS_RECORD;
		tickCounter = 0;
		MovePositionToEnd();
		return true;
	}
	return false;
}

bool RepInput::EndRecord()
{
	if (status == REPINPUT_STATUS_RECORD)
	{
		status = REPINPUT_STATUS_NONE;
		return true;
	}
	return false;
}

void RepInput::RecordAddEntry(u8 _idInput, const void * _values, size_t size)
{
	if (status != REPINPUT_STATUS_RECORD)
		return;
	RepInputEntry entry;
	entry.tick = tickCounter;
	entry.idInput = _idInput;
	for (size_t i = 0; i < size; i++)
		entry.values.push_back(reinterpret_cast<const u8*>(_values)[i]);
	inputRecords.push_back(entry);
	MovePositionToNext();
}

void RepInput::UpdateInputState(u8 _idInput, const void * _pvalues, size_t size)
{
	void *pInput = nullptr, *pOldInput = nullptr;
	switch (size)
	{
#if REPINPUT_U64_INPUT_COUNT>0
	case 8:pInput = &inputStates.u64Inputs[_idInput]; pOldInput = &lastInputStates.u64Inputs[_idInput]; break;
#endif
#if REPINPUT_U32_INPUT_COUNT>0
	case 4:pInput = &inputStates.u32Inputs[_idInput]; pOldInput = &lastInputStates.u32Inputs[_idInput]; break;
#endif
#if REPINPUT_U16_INPUT_COUNT>0
	case 2:pInput = &inputStates.u16Inputs[_idInput]; pOldInput = &lastInputStates.u16Inputs[_idInput]; break;
#endif
#if REPINPUT_U8_INPUT_COUNT>0
	case 1:pInput = &inputStates.u8Inputs[_idInput]; pOldInput = &lastInputStates.u8Inputs[_idInput]; break;
#endif
	}
	memcpy(pOldInput, pInput, size);
	memcpy(pInput, _pvalues, size);
	if (memcmp(pOldInput, pInput, size))
		RecordAddEntry(_idInput, pInput, size);
}

void RepInput::UpdateInputStateU8(u8 _idInput, u8 _value)
{
	return UpdateInputState(_idInput, &_value, sizeof(_value));
}

void RepInput::UpdateInputStateU16(u8 _idInput, u16 _value)
{
	return UpdateInputState(_idInput, &_value, sizeof(_value));
}

void RepInput::UpdateInputStateU32(u8 _idInput, u32 _value)
{
	return UpdateInputState(_idInput, &_value, sizeof(_value));
}

void RepInput::UpdateInputStateU64(u8 _idInput, u64 _value)
{
	return UpdateInputState(_idInput, &_value, sizeof(_value));
}

void RepInput::ReplayInputState()
{
	while (position < inputRecords.size() && inputRecords[position].tick <= tickCounter)
	{
		UpdateInputState(inputRecords[position].idInput, inputRecords[position].values.data(), inputRecords[position].values.size());
		MovePositionToNext();
	}
}

void RepInput::MovePositionToBegin()
{
	if (status == REPINPUT_STATUS_RECORD)
		return;
	ClearInputStates();
	position = 0;
}

void RepInput::MovePositionToEnd()
{
	if (status == REPINPUT_STATUS_NONE)
		return;
	while (position < inputRecords.size())
		MovePositionToNext();
}

void RepInput::MovePositionToNext()
{
	if (status == REPINPUT_STATUS_NONE)
		return;
	position++;
}

const RepInputEntry * RepInput::ReplayGetCurrentEntry() const
{
	if (status != REPINPUT_STATUS_NONE && position < inputRecords.size())
		return inputRecords.data() + position;
	return nullptr;
}

void RepInput::TickCounterAdd()
{
	tickCounter++;
}

const RepInputInputStates & RepInput::GetInputStates()
{
	return inputStates;
}

int RepInput::GetInputStateChangeTri(u8 _idInput, size_t size)
{
	void *pInput = nullptr, *pOldInput = nullptr;
	switch (size)
	{
#if REPINPUT_U64_INPUT_COUNT>0
	case 8:pInput = &inputStates.u64Inputs[_idInput]; pOldInput = &lastInputStates.u64Inputs[_idInput]; break;
#endif
#if REPINPUT_U32_INPUT_COUNT>0
	case 4:pInput = &inputStates.u32Inputs[_idInput]; pOldInput = &lastInputStates.u32Inputs[_idInput]; break;
#endif
#if REPINPUT_U16_INPUT_COUNT>0
	case 2:pInput = &inputStates.u16Inputs[_idInput]; pOldInput = &lastInputStates.u16Inputs[_idInput]; break;
#endif
#if REPINPUT_U8_INPUT_COUNT>0
	case 1:pInput = &inputStates.u8Inputs[_idInput]; pOldInput = &lastInputStates.u8Inputs[_idInput]; break;
#endif
	}
	if (memcmp(pInput, pOldInput, size) == 0)
		return 0;
	else if (memcmp(pInput, 0, size) == 0)
		return -1;
	else
		return 1;
}
