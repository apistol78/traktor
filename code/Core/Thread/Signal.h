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

/*! Thread signal.
 * \ingroup Core
 */
class T_DLLCLASS Signal : public IWaitable
{
public:
	Signal();

	virtual ~Signal();

	void set();

	void reset();

	virtual bool wait(int32_t timeout = -1);

private:
	void* m_handle;
};

}

