/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Theater/Act.h"

#include "Core/Class/Any.h"
#include "Core/Class/IRuntimeDispatch.h"
#include "Core/Class/Boxes/BoxedTransform.h"
#include "Core/Math/Const.h"
#include "Core/Math/Transform.h"
#include "Theater/IEntityResolver.h"
#include "Theater/Track.h"
#include "World/Entity.h"
#include "World/Entity/EventManagerComponent.h"
#include "World/IEntityComponent.h"

namespace traktor::theater
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.theater.Act", Act, Object)

Act::Act(const std::wstring& name, float start, float end, const RefArray< const Track >& tracks)
:	m_name(name)
,	m_start(start)
,	m_end(end)
,	m_tracks(tracks)
{
}

bool Act::update(const IEntityResolver& resolver, world::EventManagerComponent* eventManager, world::Entity* eventSender, const Transform& base, float timePrevious, float time, float deltaTime) const
{
	const uint32_t ntracks = (uint32_t)m_tracks.size();
	if (!ntracks)
		return false;

	const float duration = m_end - m_start;

	if (time < 0.0f || time > duration)
		return false;

	const float T = clamp(time, 0.0f, duration);

	Transform lookAtTransform;
	Transform transform;

	// Set properties, of the entities and of their components, animated by the tracks.
	for (uint32_t i = 0; i < ntracks; ++i)
	{
		world::Entity* entity = resolver.findEntity(m_tracks[i]->getEntityId());
		if (!entity)
			continue;

		for (const auto& property : m_tracks[i]->getProperties())
		{
			// A property is either of the entity itself or of one of its components.
			ITypedObject* object = entity;
			if (property.componentType != nullptr)
			{
				object = entity->getComponent(*property.componentType);
				if (object == nullptr)
					continue;
			}

			// Runtime classes do not expose the type of their properties; read it once.
			if (!property.valueTypeResolved)
			{
				property.valueType = PropertyPath::typeOfValue(property.getter->invoke(object, 0, nullptr));
				property.valueTypeResolved = true;
			}
			if (property.valueType == PropertyPath::ValueType::Invalid)
				continue;

			const PropertyPath::Value evaluated = property.path->evaluate(property.index, property.valueType, T);

			// Transforms of the entity itself are relative the performance.
			Any value;
			if (property.componentType == nullptr && property.valueType == PropertyPath::ValueType::Transform)
				value = CastAny< Transform >::set(base * evaluated.transform());
			else
				value = PropertyPath::unpack(property.valueType, evaluated);

			if (value.isVoid())
				continue;

			property.setter->invoke(object, 1, &value);
		}
	}

	// Fix-up orientation of "looking" entities.
	for (uint32_t i = 0; i < ntracks; ++i)
	{
		if (m_tracks[i]->getLookAtEntityId().isNull())
			continue;

		world::Entity* entity = resolver.findEntity(m_tracks[i]->getEntityId());
		if (!entity)
			continue;

		world::Entity* lookAtEntity = resolver.findEntity(m_tracks[i]->getLookAtEntityId());
		if (!lookAtEntity)
			continue;

		transform = entity->getTransform();
		lookAtTransform = lookAtEntity->getTransform();
		const Matrix44 m = lookAt(
			transform.translation().xyz1(),
			lookAtTransform.translation().xyz1()
		);
		transform = Transform(m.inverse());

		entity->setTransform(transform);
	}

	// Issue events which fall within the evaluated time window.
	if (eventManager != nullptr && timePrevious < time)
	{
		for (uint32_t i = 0; i < ntracks; ++i)
		{
			const AlignedVector< Track::EventKey >& events = m_tracks[i]->getEvents();
			if (events.empty())
				continue;

			// Events are issued by the given sender, if any, otherwise the track's own entity.
			world::Entity* sender = eventSender;
			if (sender == nullptr)
			{
				sender = resolver.findEntity(m_tracks[i]->getEntityId());
				if (!sender)
					continue;
			}

			const Track::Property* trackTransform = m_tracks[i]->getTransform();

			for (const auto& eventKey : events)
			{
				if (eventKey.T <= timePrevious || eventKey.T > time)
					continue;

				// Offset the event, relative the sender, to where the track is.
				Transform Toffset = Transform::identity();
				if (eventSender != nullptr && trackTransform != nullptr)
					Toffset = trackTransform->path->evaluate(trackTransform->index, PropertyPath::ValueType::Transform, clamp(eventKey.T, 0.0f, duration)).transform();

				eventManager->raise(eventKey.event, sender, Toffset);
			}
		}
	}

	return true;
}

}
