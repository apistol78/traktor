/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Theater/TrackData.h"

#include "Core/Math/MathUtils.h"
#include "Core/Math/TransformPath.h"
#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberRefArray.h"
#include "World/IEntityComponentData.h"
#include "World/IEntityEventData.h"

#include <algorithm>

namespace traktor::theater
{
namespace
{

/*! Keys of the paths of a track are told apart by their time. */
const float c_matchKeyDistance = 1.0f / 30.0f;

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.theater.TrackData", 4, TrackData, ISerializable)

void TrackData::setEntityId(const Guid& entityId)
{
	m_entityId = entityId;
}

const Guid& TrackData::getEntityId() const
{
	return m_entityId;
}

void TrackData::setLookAtEntityId(const Guid& entityId)
{
	m_lookAtEntityId = entityId;
}

const Guid& TrackData::getLookAtEntityId() const
{
	return m_lookAtEntityId;
}

const AlignedVector< TrackData::EventKey >& TrackData::getEvents() const
{
	return m_events;
}

AlignedVector< TrackData::EventKey >& TrackData::getEvents()
{
	return m_events;
}

Ref< TrackData > TrackData::stripped() const
{
	Ref< TrackData > track = new TrackData(*this);
	if (m_transform)
		track->m_transform = m_transform->stripped();
	if (m_properties)
		track->m_properties = m_properties->stripped();
	return track;
}

void TrackData::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"entityId", m_entityId, AttributePrivate());
	s >> Member< Guid >(L"lookAtEntityId", m_lookAtEntityId, AttributePrivate());

	// Transforms used to be animated by a path of their own.
	TransformPath legacyTransform;
	if (s.getVersion< TrackData >() < 3)
		s >> MemberComposite< TransformPath >(L"path", legacyTransform);

	if (s.getVersion< TrackData >() >= 1)
		s >> MemberAlignedVector< EventKey, MemberComposite< EventKey > >(L"events", m_events);

	if (s.getVersion< TrackData >() >= 4)
	{
		s >> MemberRef< PropertyPath >(L"transform", m_transform);
		s >> MemberRef< PropertyPath >(L"properties", m_properties);
	}
	else if (s.getVersion< TrackData >() >= 2)
	{
		// Every property used to be animated by a path of its own; merge them.
		RefArray< PropertyPath > legacyPaths;
		s >> MemberRefArray< PropertyPath >(L"properties", legacyPaths);

		for (auto legacyPath : legacyPaths)
		{
			if (legacyPath->getProperties().empty())
				continue;

			const PropertyPath::Property property = legacyPath->getProperties().front();
			if (property.isEntityProperty() && property.propertyName == PropertyPath::c_transformProperty)
			{
				m_transform = legacyPath;
				continue;
			}

			if (!m_properties)
				m_properties = new PropertyPath();

			const int32_t index = m_properties->addProperty(property.componentType, property.propertyName);
			for (const auto& legacyKey : legacyPath->keys())
			{
				// Properties were captured together thus their keys coincide.
				int32_t at = m_properties->getClosestKey(legacyKey.T);
				if (at < 0 || traktor::abs(m_properties->get(at).T - legacyKey.T) >= c_matchKeyDistance)
				{
					PropertyPath::Key key;
					key.T = legacyKey.T;
					key.tcb = legacyKey.tcb;
					at = (int32_t)m_properties->insert(key);
				}

				PropertyPath::Key key = m_properties->get(at);
				if (!legacyKey.values.empty())
					key.values[index] = legacyKey.values.front();

				for (auto componentData : legacyKey.componentData)
				{
					const bool have = std::any_of(key.componentData.begin(), key.componentData.end(), [&](const world::IEntityComponentData* cd) {
						return &type_of(cd) == &type_of(componentData);
					});
					if (!have)
						key.componentData.push_back(componentData);
				}

				m_properties->set(at, key);
			}
		}
	}

	if (s.getVersion< TrackData >() < 3 && legacyTransform.size() > 0)
	{
		Ref< PropertyPath > transform = new PropertyPath();
		transform->addProperty(L"", PropertyPath::c_transformProperty);

		for (const auto& legacyKey : legacyTransform.keys())
		{
			PropertyPath::Key key;
			key.T = legacyKey.T;
			key.tcb = legacyKey.tcb;
			key.values.resize(1);
			key.values[0].value = legacyKey.position.xyz1();
			key.values[0].orientation = legacyKey.orientation;
			transform->insert(key);
		}

		m_transform = transform;
	}
}

void TrackData::EventKey::serialize(ISerializer& s)
{
	s >> Member< float >(L"T", T);
	s >> MemberRef< world::IEntityEventData >(L"event", event);
}

}
