/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Theater/Track.h"

namespace traktor
{
	namespace theater
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.theater.Track", Track, Object)

Track::Track(
	world::Entity* entity,
	world::Entity* lookAtEntity,
	const TransformPath& path
)
:	m_entity(entity)
,	m_lookAtEntity(lookAtEntity)
,	m_path(path)
{
}

void Track::setEntity(world::Entity* entity)
{
	m_entity = entity;
}

world::Entity* Track::getEntity() const
{
	return m_entity;
}

void Track::setLookAtEntity(world::Entity* entity)
{
	m_lookAtEntity = entity;
}

world::Entity* Track::getLookAtEntity() const
{
	return m_lookAtEntity;
}

const TransformPath& Track::getPath() const
{
	return m_path;
}

TransformPath& Track::getPath()
{
	return m_path;
}

	}
}
