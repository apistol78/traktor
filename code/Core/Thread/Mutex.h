#ifndef traktor_Mutex_H
#define traktor_Mutex_H

#if defined(_PS3)
#	include <sys/synchronization.h>
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
#if defined(_PS3)
	sys_mutex_t m_mutex;
#else
	void* m_handle;
#endif
	bool m_existing;
};

}

#endif	// traktor_Mutex_H
