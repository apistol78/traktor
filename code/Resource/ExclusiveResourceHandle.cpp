#include "Resource/ExclusiveResourceHandle.h"

namespace traktor
{
	namespace resource
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.resource.ExclusiveResourceHandle", ExclusiveResourceHandle, IResourceHandle)

ExclusiveResourceHandle::ExclusiveResourceHandle(const TypeInfo& resourceType)
:	m_resourceType(resourceType)
{
}

void ExclusiveResourceHandle::release(void* owner) const
{
	// 2 -> 1 case; final external reference released, release
	// object and tag ourself as not being in use.
	if (getReferenceCount() == 2)
		m_object = 0;

	Object::release(owner);
}

void ExclusiveResourceHandle::replace(Object* object)
{
	m_object = object;
}

Object* ExclusiveResourceHandle::get() const
{
	return m_object;
}

void ExclusiveResourceHandle::flush()
{
	m_object = 0;
}

	}
}
