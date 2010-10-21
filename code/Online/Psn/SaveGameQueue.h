#ifndef traktor_online_SaveGameQueue_H
#define traktor_online_SaveGameQueue_H

#include <np.h>
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Thread/Signal.h"

namespace traktor
{

class ISerializable;
class Thread;

	namespace online
	{

class ISaveGame;
class ISaveGameQueueTask;

class SaveGameQueue : public Object
{
	T_RTTI_CLASS;

public:
	bool create(int32_t requiredTrophySizeKB);

	void destroy();

	Ref< ISaveGame > createSaveGame(const std::wstring& name, ISerializable* attachment);

	bool registerTrophyContext(SceNpTrophyContext trophyContext, SceNpTrophyHandle trophyHandle);

	bool getAvailableSaveGames(RefArray< ISaveGame >& outSaveGames);

private:
	Thread* m_thread;
	Semaphore m_queueLock;
	Signal m_queuedSignal;
	RefArray< ISaveGameQueueTask > m_queue;
	RefArray< ISaveGame > m_saveGames;
	int32_t	m_hddFreeSpace;
	bool m_saveGameExists;

	void flush();

	void threadQueue();
};

	}
}

#endif	// traktor_online_SaveGameQueue_H
