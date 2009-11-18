#ifndef traktor_Object_H
#define traktor_Object_H

#include "Core/IRefCount.h"
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

/*
*/
class T_DLLCLASS Object
:	public IRefCount
,	public ITypedObject
{
	T_RTTI_CLASS;

public:
	Object();

	virtual ~Object();

	virtual void addRef() const;

	virtual void release() const;

	void* operator new (size_t size);

	void operator delete (void* ptr);

protected:
	void* operator new[] (size_t);

	void operator delete[] (void*);

private:
	mutable AtomicRefCount m_refCount;
	bool m_heap;
};	

}

#endif	// traktor_Object_H
