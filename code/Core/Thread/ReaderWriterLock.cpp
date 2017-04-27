/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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
	bool result = false;
	for (;;)
	{
		if (!m_lock.wait(timeout))
			return false;

		if (m_writer <= 0)
		{
			++m_reader;
			result = true;
		}

		m_lock.release();

		if (result)
			break;
	}
	return result;
}

void ReaderWriterLock::releaseReader()
{
	Acquire< Semaphore > lock(m_lock);
	T_ASSERT (m_writer == 0);
	--m_reader;
}

bool ReaderWriterLock::acquireWriter(int32_t timeout)
{
	bool result = false;
	for (;;)
	{
		if (!m_lock.wait(timeout))
			return false;

		if (m_writer <= 0 && m_reader <= 0)
		{
			++m_writer;
			result = true;
		}

		m_lock.release();

		if (result)
			break;
	}
	return result;
}

void ReaderWriterLock::releaseWriter()
{
	Acquire< Semaphore > lock(m_lock);
	--m_writer;
	T_ASSERT (m_writer == 0);
}

}
