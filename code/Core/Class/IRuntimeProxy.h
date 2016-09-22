#ifndef traktor_IRuntimeProxy_H
#define traktor_IRuntimeProxy_H

#include "Core/Object.h"
#include "Core/Class/Any.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Runtime proxy interface.
 * \ingroup Core
 *
 * A runtime proxy could be considered as a wrapper
 * of another, embedded, object which gets automatically
 * unwrapped if proxy is cast to embedded object's type.
 */
class T_DLLCLASS IRuntimeProxy : public Object
{
	T_RTTI_CLASS;

public:
	virtual Object* getProxyObject() = 0;

	template < typename As >
	static As* getObject(const Any& any)
	{
		if (!any.isObject())
			return 0;

		if (is_a< As >(any.getObjectUnsafe()))
			return checked_type_cast< As* >(any.getObjectUnsafe());
		
		IRuntimeProxy* proxy = dynamic_type_cast< IRuntimeProxy* >(any.getObjectUnsafe());
		if (!proxy)
			return 0;

		return (As*)proxy->getProxyObject();
	}
};

}

#endif	// traktor_IRuntimeProxy_H

