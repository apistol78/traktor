#pragma once

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

/*! Critical section primitive.
 * \ingroup Core
 */
class T_DLLCLASS CriticalSection : public IWaitable
{
public:
	CriticalSection();

	virtual ~CriticalSection();

	virtual bool wait(int32_t timeout = -1);

	void release();

private:
	void* m_handle;
};

}

