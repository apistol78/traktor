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
	Object()
	{
	}

	Object(const Object& object)
	{
	}

	virtual void addRef(void* owner) const
#if !defined(_DEBUG)
	{
		++m_refCount;
	}
#else
	;
#endif

	virtual void release(void* owner) const
#if !defined(_DEBUG)
	{
		if (--m_refCount == 0)
			finalRelease();
	}
#else
	;
#endif

	int32_t getReferenceCount() const
#if !defined(_DEBUG)
	{
		return m_refCount;
	}
#else
	;
#endif

	void* operator new (size_t size);

	void* operator new (size_t size, void* memory);

	void operator delete (void* ptr);

	void operator delete (void* ptr, void* memory);

	Object& operator = (const Object& object)
	{
		return *this;
	}

	static void setReferenceDebugger(IObjectRefDebugger* refDebugger);

	static int32_t getHeapObjectCount();

private:
	static IObjectRefDebugger* ms_refDebugger;
	mutable AtomicRefCount m_refCount;

	void finalRelease() const;
};	

}

#endif	// traktor_Object_H
