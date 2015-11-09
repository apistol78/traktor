#ifndef traktor_Mutex_H
#define traktor_Mutex_H

#if defined(__PS3__)
#	include <sys/synchronization.h>
#elif defined(__PS4__)
#	include <kernel.h>
#endif
#include <string>
#include "Core/Guid.h"
#include "Core/Thread/IWaitable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Mutually exclusive primitive.
 * \ingroup Core
 */
class T_DLLCLASS Mutex : public IWaitable
{
public:
	Mutex();
	
	Mutex(const Guid& id);
	
	virtual ~Mutex();

	virtual bool wait(int32_t timeout = -1);
	
	void release();

	bool existing() const;

private:
#if defined(__PS3__)
	sys_mutex_t m_mutex;
#elif defined(__PS4__)
	ScePthreadMutex m_mutex;
#else
	void* m_handle;
#endif
	bool m_existing;
};

}

#endif	// traktor_Mutex_H
