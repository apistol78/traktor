/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberSmallMap.h"
#include "World/Entity/EventSetComponent.h"
#include "World/Entity/EventSetComponentData.h"
#include "World/IEntityBuilder.h"
#include "World/IEntityEventData.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.EventSetComponentData", 0, EventSetComponentData, IEntityComponentData)

Ref< EventSetComponent > EventSetComponentData::createComponent(const IEntityBuilder* entityBuilder) const
{
	Ref< EventSetComponent > eventSet = new EventSetComponent();
	for (auto eventData : m_eventData)
	{
		if (!eventData.second)
			continue;

		Ref< world::IEntityEvent > event = entityBuilder->create(eventData.second);
		if (!event)
			continue;

		eventSet->m_events.insert(std::make_pair(
			eventData.first,
			event
		));
	}
	return eventSet;
}

int32_t EventSetComponentData::getOrdinal() const
{
	return 0;
}

void EventSetComponentData::setTransform(const EntityData* owner, const Transform& transform)
{
}

void EventSetComponentData::serialize(ISerializer& s)
{
	s >> MemberSmallMap<
		std::wstring,
		Ref< IEntityEventData >,
		Member< std::wstring >,
		MemberRef< IEntityEventData >
	>(L"eventData", m_eventData);
}

}
