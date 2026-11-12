/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Heightfield/HeightfieldResource.h"

#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor::hf
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.HeightfieldResource", 2, HeightfieldResource, ISerializable)

void HeightfieldResource::serialize(ISerializer& s)
{
	T_ASSERT(s.getVersion() >= 2);
	s >> Member< Vector4 >(L"worldExtent", m_worldExtent);
	s >> Member< Vector4 >(L"worldCenter", m_worldCenter);
}

}
