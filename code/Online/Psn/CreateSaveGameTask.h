#ifndef traktor_online_CreateSaveGameTask_H
#define traktor_online_CreateSaveGameTask_H

#include <sysutil/sysutil_savedata.h>
#include "Online/Psn/ISaveGameQueueTask.h"

namespace traktor
{

class ISerializable;

	namespace online
	{

class CreateSaveGameTask : public ISaveGameQueueTask
{
	T_RTTI_CLASS;

public:
	static Ref< ISaveGameQueueTask > create(const std::wstring& name, const ISerializable* attachment);

	virtual bool execute();

private:
	std::vector< uint8_t > m_saveBuffer;
	bool m_saveBufferPending;

	static void callbackSaveStat(CellSaveDataCBResult* cbResult, CellSaveDataStatGet* get, CellSaveDataStatSet* set);

	static void callbackSaveFile(CellSaveDataCBResult* cbResult, CellSaveDataFileGet* get, CellSaveDataFileSet* set);
};

	}
}

#endif	// traktor_online_CreateSaveGameTask_H
