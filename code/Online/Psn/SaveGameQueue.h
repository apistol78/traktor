#ifndef traktor_online_SaveGameQueue_H
#define traktor_online_SaveGameQueue_H

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
	bool create();

	void destroy();

	Ref< ISaveGame > createSaveGame(const std::wstring& name, ISerializable* attachment);

	bool getAvailableSaveGames(RefArray< ISaveGame >& outSaveGames);

private:
	Thread* m_thread;
	Semaphore m_queueLock;
	Signal m_queuedSignal;
	RefArray< ISaveGameQueueTask > m_queue;
	RefArray< ISaveGame > m_saveGames;

	void flush();

	void threadQueue();
};

	}
}

#endif	// traktor_online_SaveGameQueue_H
