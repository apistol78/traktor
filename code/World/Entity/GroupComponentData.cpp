/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "World/EntityData.h"
#include "World/Entity/GroupComponentData.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.GroupComponentData", 0, GroupComponentData, IEntityComponentData)

GroupComponentData::GroupComponentData(const RefArray< EntityData >& entityData)
:	m_entityData(entityData)
{
}

void GroupComponentData::addEntityData(EntityData* entityData)
{
	T_ASSERT(std::find(m_entityData.begin(), m_entityData.end(), entityData) == m_entityData.end());
	m_entityData.push_back(entityData);
}

void GroupComponentData::removeEntityData(EntityData* entityData)
{
	auto i = std::find(m_entityData.begin(), m_entityData.end(), entityData);
	if (i != m_entityData.end())
		m_entityData.erase(i);
}

void GroupComponentData::removeAllEntityData()
{
	m_entityData.resize(0);
}

void GroupComponentData::setEntityData(const RefArray< EntityData >& entityData)
{
	m_entityData = entityData;
}

RefArray< EntityData >& GroupComponentData::getEntityData()
{
	return m_entityData;
}

const RefArray< EntityData >& GroupComponentData::getEntityData() const
{
	return m_entityData;
}

void GroupComponentData::setTransform(const EntityData* owner, const Transform& transform)
{
	Transform invTransform = owner->getTransform().inverse();
	for (auto entityData : m_entityData)
	{
		if (entityData != nullptr)
		{
			Transform currentTransform = entityData->getTransform();
			Transform Tlocal = invTransform * currentTransform;
			Transform Tworld = transform * Tlocal;
			entityData->setTransform(Tworld);
		}
	}
}

void GroupComponentData::serialize(ISerializer& s)
{
    s >> MemberRefArray< EntityData >(L"entityData", m_entityData, AttributePrivate());
}

	}
}
