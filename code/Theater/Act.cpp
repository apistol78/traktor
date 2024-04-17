/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Const.h"
#include "Theater/Act.h"
#include "Theater/Track.h"
#include "World/Entity.h"
#include "World/World.h"

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

bool Act::update(world::World* world, float time, float deltaTime) const
{
	const uint32_t ntracks = (uint32_t)m_tracks.size();
	if (!ntracks)
		return false;

	const float duration = m_end - m_start;

	if (time < 0.0f || time > duration)
		return false;

	TransformPath::Key key;
	Transform lookAtTransform;
	Transform transform;

	// Calculate transforms.
	for (uint32_t i = 0; i < ntracks; ++i)
	{
		world::Entity* entity = world->getEntity(m_tracks[i]->getEntityId());
		if (!entity)
			continue;

		const TransformPath& path = m_tracks[i]->getPath();

		key = path.evaluate(clamp(time, 0.0f, duration), false);
		transform = key.transform();

		entity->setTransform(transform);
	}

	// Fix-up orientation of "looking" entities.
	for (uint32_t i = 0; i < ntracks; ++i)
	{
		if (m_tracks[i]->getLookAtEntityId().isNull())
			continue;

		world::Entity* entity = world->getEntity(m_tracks[i]->getEntityId());
		if (!entity)
			continue;

		world::Entity* lookAtEntity = world->getEntity(m_tracks[i]->getLookAtEntityId());
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

	return true;
}

}
