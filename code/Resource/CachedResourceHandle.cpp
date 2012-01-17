#include "Resource/CachedResourceHandle.h"

namespace traktor
{
	namespace resource
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.resource.CachedResourceHandle", CachedResourceHandle, IResourceHandle)

CachedResourceHandle::CachedResourceHandle(const TypeInfo& resourceType)
:	m_resourceType(resourceType)
{
}

void CachedResourceHandle::replace(Object* object)
{
	m_object = object;
}

Object* CachedResourceHandle::get() const
{
	return m_object;
}

void CachedResourceHandle::flush()
{
	m_object = 0;
}

	}
}
