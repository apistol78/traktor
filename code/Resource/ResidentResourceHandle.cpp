/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
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
