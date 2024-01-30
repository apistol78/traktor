/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
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

namespace traktor::world
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

void GroupComponentData::addEntityData(EntityData* insertAfter, EntityData* entityData)
{
	auto it = std::find(m_entityData.begin(), m_entityData.end(), insertAfter);
	if (it != m_entityData.end())
		m_entityData.insert(it + 1, entityData);
	else
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

int32_t GroupComponentData::getOrdinal() const
{
	return -10000;
}

void GroupComponentData::setTransform(const EntityData* owner, const Transform& transform)
{
	const Transform invTransform = owner->getTransform().inverse();
	for (auto entityData : m_entityData)
	{
		if (entityData != nullptr)
		{
			const Transform currentTransform = entityData->getTransform();
			const Transform Tlocal = invTransform * currentTransform;
			const Transform Tworld = transform * Tlocal;
			entityData->setTransform(Tworld);
		}
	}
}

void GroupComponentData::serialize(ISerializer& s)
{
    s >> MemberRefArray< EntityData >(L"entityData", m_entityData, AttributePrivate());
}

}
