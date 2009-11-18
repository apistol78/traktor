#ifndef traktor_Semaphore_H
#define traktor_Semaphore_H

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

/*! \brief Blocking semaphore primitive.
 * \ingroup Core
 */
class T_DLLCLASS Semaphore : public IWaitable
{
public:
	Semaphore();
	
	~Semaphore();

	virtual bool wait(int32_t timeout = -1);
	
	void release();

private:
	void* m_handle;
};

}

#endif	// traktor_Semaphore_H
