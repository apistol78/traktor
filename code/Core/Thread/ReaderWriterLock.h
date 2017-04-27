/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ReaderWriterLock_H
#define traktor_ReaderWriterLock_H

#include "Core/Thread/Semaphore.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Multiple reader, single writer lock.
 * \ingroup Core
 */
class T_DLLCLASS ReaderWriterLock
{
public:
	struct AcquireReader
	{
		ReaderWriterLock& m_lock;

		AcquireReader(ReaderWriterLock& lock)
		:	m_lock(lock)
		{
			m_lock.acquireReader();
		}

		~AcquireReader()
		{
			m_lock.releaseReader();
		}
	};

	struct AcquireWriter
	{
		ReaderWriterLock& m_lock;

		AcquireWriter(ReaderWriterLock& lock)
		:	m_lock(lock)
		{
			m_lock.acquireWriter();
		}

		~AcquireWriter()
		{
			m_lock.releaseWriter();
		}
	};

	ReaderWriterLock();

	bool acquireReader(int32_t timeout = -1);

	void releaseReader();

	bool acquireWriter(int32_t timeout = -1);

	void releaseWriter();

private:
	Semaphore m_lock;
	int32_t m_reader;
	int32_t m_writer;
};

}

#endif	// traktor_ReaderWriterLock_H
