#include "Resource/ExplicitResourceHandle.h"

namespace traktor
{
	namespace resource
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.resource.ExplicitResourceHandle", ExplicitResourceHandle, IResourceHandle)

ExplicitResourceHandle::ExplicitResourceHandle(Object* object)
:	m_object(object)
{
}

void ExplicitResourceHandle::replace(Object* object)
{
	m_object = object;
}

Object* ExplicitResourceHandle::get() const
{
	return m_object;
}

void ExplicitResourceHandle::flush()
{
	m_object = 0;
}

	}
}
