#include "Resource/ResourceHandle.h"

namespace traktor
{
	namespace resource
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.resource.ResourceHandle", ResourceHandle, IResourceHandle)

ResourceHandle::ResourceHandle(const TypeInfo& resourceType)
:	m_resourceType(resourceType)
{
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
