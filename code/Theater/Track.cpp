/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Theater/Track.h"

namespace traktor::theater
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.theater.Track", Track, Object)

Track::Track(
	const Guid& entityId,
	const Guid& lookAtEntityId,
	const AlignedVector< EventKey >& events,
	const AlignedVector< Property >& properties)
	: m_entityId(entityId)
	, m_lookAtEntityId(lookAtEntityId)
	, m_events(events)
	, m_properties(properties)
{
	for (int32_t i = 0; i < (int32_t)m_properties.size(); ++i)
	{
		const Property& property = m_properties[i];
		if (property.componentType != nullptr)
			continue;

		const PropertyPath::Property& p = property.path->getProperties()[property.index];
		if (p.propertyName == PropertyPath::c_transformProperty)
		{
			m_transform = i;
			break;
		}
	}
}

const Guid& Track::getEntityId() const
{
	return m_entityId;
}

const Guid& Track::getLookAtEntityId() const
{
	return m_lookAtEntityId;
}

const AlignedVector< Track::EventKey >& Track::getEvents() const
{
	return m_events;
}

const AlignedVector< Track::Property >& Track::getProperties() const
{
	return m_properties;
}

const Track::Property* Track::getTransform() const
{
	return (m_transform >= 0) ? &m_properties[m_transform] : nullptr;
}

}
