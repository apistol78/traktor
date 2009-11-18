#ifndef traktor_Mutex_H
#define traktor_Mutex_H

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
	
	~Mutex();

	virtual bool wait(int32_t timeout = -1);
	
	void release();

	bool existing() const;

private:
	void* m_handle;
	bool m_existing;
};

}

#endif	// traktor_Mutex_H
