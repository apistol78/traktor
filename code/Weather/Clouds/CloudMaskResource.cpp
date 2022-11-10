/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Weather/Clouds/CloudMaskResource.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace weather
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.terrain.CloudMaskResource", 0, CloudMaskResource, ISerializable)

CloudMaskResource::CloudMaskResource(int32_t size)
:	m_size(size)
{
}

int32_t CloudMaskResource::getSize() const
{
	return m_size;
}

void CloudMaskResource::serialize(ISerializer& s)
{
	s >> Member< int32_t >(L"size", m_size);
}

	}
}
