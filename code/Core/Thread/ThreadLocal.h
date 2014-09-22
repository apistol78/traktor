#ifndef traktor_ThreadLocal_H
#define traktor_ThreadLocal_H

#include "Core/Object.h"

namespace traktor
{

/*! \brief Thread local storage.
 * \ingroup Core
 *
 * Provide thread unique storage of a pointer
 * using TLS (Thread Local Storage).
 */
class T_DLLCLASS ThreadLocal : public Object
{
	T_RTTI_CLASS;

public:
	ThreadLocal();

	virtual ~ThreadLocal();

	void set(void* ptr);

	void* get() const;

private:
	uint64_t m_handle;
};

}

#endif	// traktor_ThreadLocal_H
