#include "Core/Thread/ReaderWriterLock.h"

namespace traktor
{

bool ReaderWriterLock::acquireReader(int32_t timeout)
{
	m_lock.lock_shared();
	return true;
}

void ReaderWriterLock::releaseReader()
{
	m_lock.unlock_shared();
}

bool ReaderWriterLock::acquireWriter(int32_t timeout)
{
	m_lock.lock();
	return true;
}

void ReaderWriterLock::releaseWriter()
{
	m_lock.unlock();
}

}
