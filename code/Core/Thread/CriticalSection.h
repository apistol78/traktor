#ifndef traktor_CriticalSection_H
#define traktor_CriticalSection_H

#include "Core/Config.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Critical section primitive.
 * \ingroup Core
 */
class T_DLLCLASS CriticalSection
{
public:
	CriticalSection();
	
	~CriticalSection();

	bool acquire(int timeout = -1);

	void release();

private:
	void* m_handle;
};

}

#endif	// traktor_CriticalSection_H
