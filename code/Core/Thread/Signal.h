#ifndef traktor_Signal_H
#define traktor_Signal_H

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

/*! \brief Thread signal.
 * \ingroup Core
 */
class T_DLLCLASS Signal
{
public:
	Signal();

	virtual ~Signal();
	
	void set();

	void reset();

	bool wait(int timeout = -1);
	
private:
	void* m_handle;
};

}

#endif	// traktor_Signal_H
