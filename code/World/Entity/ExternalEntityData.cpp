/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Resource/Member.h"
#include "World/Entity/ExternalEntityData.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.ExternalEntityData", 1, ExternalEntityData, EntityData)

ExternalEntityData::ExternalEntityData(const resource::Id< EntityData >& entityData)
:	m_entityData(entityData)
{
}

void ExternalEntityData::setEntityData(const resource::Id< EntityData >& entityData)
{
	m_entityData = entityData;
}

const resource::Id< EntityData >& ExternalEntityData::getEntityData() const
{
	return m_entityData;
}

void ExternalEntityData::serialize(ISerializer& s)
{
	EntityData::serialize(s);

	if (s.getVersion() >= 1)
		s >> resource::Member< EntityData >(L"entityData", m_entityData);
	else
		s >> resource::Member< EntityData >(L"guid", m_entityData);
}

}
