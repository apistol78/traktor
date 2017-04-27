/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Resource/ExplicitResourceHandle.h"

namespace traktor
{
	namespace resource
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.resource.ExplicitResourceHandle", ExplicitResourceHandle, ResourceHandle)

ExplicitResourceHandle::ExplicitResourceHandle(Object* object)
{
	replace(object);
}

	}
}
