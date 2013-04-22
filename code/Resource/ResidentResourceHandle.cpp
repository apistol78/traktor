#include "Resource/ResidentResourceHandle.h"

namespace traktor
{
	namespace resource
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.resource.ResidentResourceHandle", ResidentResourceHandle, IResourceHandle)

ResidentResourceHandle::ResidentResourceHandle(const TypeInfo& resourceType, bool persistent)
:	m_resourceType(resourceType)
,	m_persistent(persistent)
{
}

void ResidentResourceHandle::replace(Object* object)
{
	m_object = object;
}

Object* ResidentResourceHandle::get() const
{
	return m_object;
}

void ResidentResourceHandle::flush()
{
	m_object = 0;
}

	}
}
