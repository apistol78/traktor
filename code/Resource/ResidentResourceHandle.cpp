#include "Resource/ResidentResourceHandle.h"

namespace traktor
{
	namespace resource
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.resource.ResidentResourceHandle", ResidentResourceHandle, ResourceHandle)

ResidentResourceHandle::ResidentResourceHandle(const TypeInfo& resourceType, bool persistent)
:	m_resourceType(resourceType)
,	m_persistent(persistent)
{
}

	}
}
