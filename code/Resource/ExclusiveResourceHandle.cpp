/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Resource/ExclusiveResourceHandle.h"

namespace traktor
{
	namespace resource
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.resource.ExclusiveResourceHandle", ExclusiveResourceHandle, ResourceHandle)

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

	}
}
