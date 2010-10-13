#ifndef traktor_online_GetAvailableSaveGamesTask_H
#define traktor_online_GetAvailableSaveGamesTask_H

#include <sysutil/sysutil_savedata.h>
#include "Core/RefArray.h"
#include "Online/Psn/ISaveGameQueueTask.h"

namespace traktor
{

class ISerializable;

	namespace online
	{

class ISaveGame;

class GetAvailableSaveGamesTask : public ISaveGameQueueTask
{
	T_RTTI_CLASS;

public:
	static Ref< ISaveGameQueueTask > create(RefArray< ISaveGame >& outSaveGames);

	virtual bool execute();

private:
	RefArray< ISaveGame >* m_outSaveGames;
	std::vector< uint8_t > m_loadBuffer;
	bool m_loadBufferPending;

	static void callbackLoadFixed(CellSaveDataCBResult* cbResult, CellSaveDataListGet* get, CellSaveDataFixedSet* set);

	static void callbackLoadStat(CellSaveDataCBResult* cbResult, CellSaveDataStatGet* get, CellSaveDataStatSet* set);

	static void callbackLoadFile(CellSaveDataCBResult* cbResult, CellSaveDataFileGet* get, CellSaveDataFileSet* set);
};

	}
}

#endif	// traktor_online_GetAvailableSaveGamesTask_H
