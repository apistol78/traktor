/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Entity/EntityStreamingComponentData.h"

#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAabb.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRefArray.h"
#include "World/EntityData.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.EntityStreamingComponentData", 0, EntityStreamingComponentData, IWorldComponentData)

void EntityStreamingComponentData::serialize(ISerializer& s)
{
	s >> Member< float >(L"loadDistance", m_loadDistance, AttributeRange(0.0f));
	s >> Member< float >(L"unloadDistance", m_unloadDistance, AttributeRange(0.0f));
	s >> MemberAlignedVector< Cell, MemberComposite< Cell > >(L"cells", m_cells, AttributePrivate());
}

void EntityStreamingComponentData::Cell::serialize(ISerializer& s)
{
	s >> MemberAabb3(L"boundingBox", boundingBox);
	s >> MemberRefArray< EntityData >(L"entityData", entityData);
}

}
