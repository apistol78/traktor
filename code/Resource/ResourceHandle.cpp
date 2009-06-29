#include "Resource/ResourceHandle.h"

namespace traktor
{
	namespace resource
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.resource.ResourceHandle", ResourceHandle, IResourceHandle)

void ResourceHandle::replace(Object* object)
{
	m_object = object;
}

Object* ResourceHandle::get()
{
	return m_object;
}

void ResourceHandle::flush()
{
	m_object = 0;
}

	}
}
