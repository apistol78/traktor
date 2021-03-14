#include "Core/Thread/ReaderWriterLock.h"

namespace traktor
{

bool ReaderWriterLock::acquireReader(int32_t timeout)
{
#if !defined(__EMSCRIPTEN__)
	m_lock.lock_shared();
#endif
	return true;
}

void ReaderWriterLock::releaseReader()
{
#if !defined(__EMSCRIPTEN__)
	m_lock.unlock_shared();
#endif
}

bool ReaderWriterLock::acquireWriter(int32_t timeout)
{
#if !defined(__EMSCRIPTEN__)
	m_lock.lock();
#endif
	return true;
}

void ReaderWriterLock::releaseWriter()
{
#if !defined(__EMSCRIPTEN__)
	m_lock.unlock();
#endif
}

}
