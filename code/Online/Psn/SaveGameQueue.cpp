#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Online/Psn/CreateSaveGameTask.h"
#include "Online/Psn/GetAvailableSaveGamesTask.h"
#include "Online/Psn/SaveGamePsn.h"
#include "Online/Psn/SaveGameQueue.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.SaveGameQueue", SaveGameQueue, Object)

bool SaveGameQueue::create()
{
	m_thread = ThreadManager::getInstance().create(makeFunctor(this, &SaveGameQueue::threadQueue), L"Save game queue");
	if (!m_thread || !m_thread->start())
		return false;

	// Get stored save games.
	Ref< ISaveGameQueueTask > task = GetAvailableSaveGamesTask::create(m_saveGames);
	if (task)
	{
		m_queueLock.wait();
		m_queue.push_back(task);
		m_queuedSignal.set();
		m_queueLock.release();

		// \fixme Wait for save games in first getAvailableSaveGames.
		flush();
	}
	
	return true;
}

void SaveGameQueue::destroy()
{
	flush();

	if (m_thread)
	{
		m_thread->stop();
		ThreadManager::getInstance().destroy(m_thread);
		m_thread = 0;
	}
}

Ref< ISaveGame > SaveGameQueue::createSaveGame(const std::wstring& name, ISerializable* attachment)
{
	Ref< ISaveGameQueueTask > task = CreateSaveGameTask::create(name, attachment);
	if (!task)
		return false;

	m_queueLock.wait();
	m_queue.push_back(task);
	m_queuedSignal.set();
	m_queueLock.release();

	// Add another save game to our list.
	Ref< ISaveGame > saveGame = new SaveGamePsn(name, attachment);
	m_saveGames.push_back(saveGame);

	return saveGame;
}

bool SaveGameQueue::getAvailableSaveGames(RefArray< ISaveGame >& outSaveGames)
{
	outSaveGames = m_saveGames;
	return true;
}

void SaveGameQueue::flush()
{
	Thread* currentThread = ThreadManager::getInstance().getCurrentThread();
	for (bool queueEmpty = false; !queueEmpty;)
	{
		m_queueLock.wait();
		queueEmpty = m_queue.empty();
		m_queueLock.release();
		if (!queueEmpty)
			currentThread->sleep(100);
	}
}

void SaveGameQueue::threadQueue()
{
	while (!m_thread->stopped())
	{
		if (!m_queuedSignal.wait(200))
			continue;

		if (!m_queueLock.wait(100))
			continue;

		if (m_queue.empty())
		{
			// No more queued tasks.
			m_queuedSignal.reset();
			m_queueLock.release();
			continue;
		}

		Ref< ISaveGameQueueTask > task = m_queue.front();
		m_queue.pop_front();

		m_queueLock.release();

		if (task)
			task->execute();
	}
}

	}
}
