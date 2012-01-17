#include "Resource/UncachedResourceHandle.h"

namespace traktor
{
	namespace resource
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.resource.UncachedResourceHandle", UncachedResourceHandle, IResourceHandle)

UncachedResourceHandle::UncachedResourceHandle(const TypeInfo& resourceType)
:	m_resourceType(resourceType)
,	m_inUse(true)
{
}

void UncachedResourceHandle::release(void* owner) const
{
	// 2 -> 1 case; final external reference released, release
	// object and tag ourself as not being in use.
	if (getReferenceCount() == 2)
	{
		m_object = 0;
		m_inUse = false;
	}

	Object::release(owner);
}

void UncachedResourceHandle::replace(Object* object)
{
	m_object = object;
}

Object* UncachedResourceHandle::get() const
{
	return m_object;
}

void UncachedResourceHandle::flush()
{
	m_object = 0;
}

	}
}
