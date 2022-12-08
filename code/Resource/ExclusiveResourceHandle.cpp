/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Resource/ExclusiveResourceHandle.h"

namespace traktor::resource
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
		m_object = nullptr;

	Object::release(owner);
}

}
