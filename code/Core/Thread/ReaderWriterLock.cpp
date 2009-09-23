#include "Core/Thread/ReaderWriterLock.h"
#include "Core/Thread/Acquire.h"

namespace traktor
{

ReaderWriterLock::ReaderWriterLock()
:	m_reader(0)
,	m_writer(0)
{
}

bool ReaderWriterLock::acquireReader(int32_t timeout)
{
	for (;;)
	{
		if (!m_lock.acquire(timeout))
			return false;

		if (m_writer > 0)
			continue;

		++m_reader;

		m_lock.release();
		break;
	}
	return true;
}

void ReaderWriterLock::releaseReader()
{
	Acquire< Semaphore > lock(m_lock);
	T_ASSERT (m_writer == 0);
	--m_reader;
}

bool ReaderWriterLock::acquireWriter(int32_t timeout)
{
	for (;;)
	{
		if (!m_lock.acquire(timeout))
			return false;

		if (m_writer > 0 || m_reader > 0)
			continue;

		++m_writer;

		m_lock.release();
		break;
	}
	return true;
}

void ReaderWriterLock::releaseWriter()
{
	Acquire< Semaphore > lock(m_lock);
	--m_writer;
	T_ASSERT (m_writer == 0);
}

}
