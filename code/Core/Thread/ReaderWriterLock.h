#pragma once

#include <shared_mutex>
#include "Core/Config.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! Multiple reader, single writer lock.
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

	bool acquireReader(int32_t timeout = -1);

	void releaseReader();

	bool acquireWriter(int32_t timeout = -1);

	void releaseWriter();

private:
	std::shared_timed_mutex m_lock;
};

}

