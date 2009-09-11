#ifndef traktor_Event_H
#define traktor_Event_H

#include "Core/Thread/IWaitable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Thread signal event.
 * \ingroup Core
 */
class T_DLLCLASS Event : public IWaitable
{
public:
	Event();

	virtual ~Event();
	
	void pulse(int count = 1);

	void broadcast();

	void reset();

	virtual bool wait(int32_t timeout = -1);
	
private:
	void* m_handle;
};

}

#endif	// traktor_Event_H
