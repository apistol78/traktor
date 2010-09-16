#ifndef traktor_Object_H
#define traktor_Object_H

#include "Core/Rtti/ITypedObject.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

struct IObjectRefDebugger;

/*! \brief Managed object base class.
 * \ingroup Core
 */
class T_DLLCLASS Object : public ITypedObject
{
	T_RTTI_CLASS;

public:
	virtual void addRef(void* owner) const;

	virtual void release(void* owner) const;

	void* operator new (size_t size);

	void operator delete (void* ptr);

	int32_t getReferenceCount() const;

	static void setReferenceDebugger(IObjectRefDebugger* refDebugger);

private:
	static IObjectRefDebugger* ms_refDebugger;
	mutable AtomicRefCount m_refCount;
};	

}

#endif	// traktor_Object_H
