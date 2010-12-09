#include "Resource/ResourceHandle.h"

namespace traktor
{
	namespace resource
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.resource.ResourceHandle", ResourceHandle, IResourceHandle)

ResourceHandle::ResourceHandle(const TypeInfo& resourceType, bool cacheable)
:	m_resourceType(resourceType)
,	m_cacheable(cacheable)
{
}

void ResourceHandle::release(void* owner) const
{
	// 2 -> 1 case; final external reference released, flush ourself
	// in case we're a non-cacheable resource.
	if (!m_cacheable && getReferenceCount() == 2)
		m_object = 0;

	Object::release(owner);
}

void ResourceHandle::replace(Object* object)
{
	m_object = object;
}

Object* ResourceHandle::get() const
{
	return m_object;
}

void ResourceHandle::flush()
{
	m_object = 0;
}

const TypeInfo& ResourceHandle::getResourceType() const
{
	return m_resourceType;
}

	}
}
