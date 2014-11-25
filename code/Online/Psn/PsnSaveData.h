#ifndef traktor_online_PsnSaveData_H
#define traktor_online_PsnSaveData_H

#include <sysutil/sysutil_savedata.h>
#include "Online/Provider/ISaveDataProvider.h"

namespace traktor
{
	class Thread;
}

namespace traktor
{
	namespace online
	{

class PsnSaveData : public ISaveDataProvider
{
	T_RTTI_CLASS;

public:
	PsnSaveData(int32_t excessSpaceNeededKB, const uint8_t* saveIconBuffer, int32_t saveIconSize);

	~PsnSaveData();
	virtual bool enumerate(std::set< std::wstring >& outSaveDataIds);

	virtual bool get(const std::wstring& saveDataId, Ref< ISerializable >& outAttachment);

	virtual bool set(const std::wstring& saveDataId, const SaveDataDesc& saveDataDesc, const ISerializable* attachment, bool replace);

	virtual bool remove(const std::wstring& saveDataId);

private:
	int32_t m_excessSpaceNeededKB;
	int32_t m_hddFreeSpaceKB;
	int32_t m_spaceNeededKB;
	int32_t m_currentSavedataSizeKB;
	const uint8_t* m_saveIconBuffer;
	int32_t m_saveIconSize;

	Thread* m_threadDialog;

	void dialogThread();
	
	static void callbackEnumFixed(CellSaveDataCBResult* cbResult, CellSaveDataListGet* get, CellSaveDataFixedSet* set);

	static void callbackEnumStat(CellSaveDataCBResult* cbResult, CellSaveDataStatGet* get, CellSaveDataStatSet* set);

	static void callbackEnumFile(CellSaveDataCBResult* cbResult, CellSaveDataFileGet* get, CellSaveDataFileSet* set);

	static void callbackLoadStat(CellSaveDataCBResult* cbResult, CellSaveDataStatGet* get, CellSaveDataStatSet* set);

	static void callbackLoadFile(CellSaveDataCBResult* cbResult, CellSaveDataFileGet* get, CellSaveDataFileSet* set);

	static void callbackSaveStat(CellSaveDataCBResult* cbResult, CellSaveDataStatGet* get, CellSaveDataStatSet* set);

	static void callbackSaveFile(CellSaveDataCBResult* cbResult, CellSaveDataFileGet* get, CellSaveDataFileSet* set);
};

	}
}

#endif	// traktor_online_PsnSaveData_H
