/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Theater/TheaterEntityComponent.h"

#include "Theater/GroupEntityResolver.h"
#include "Theater/Performance.h"
#include "World/Entity.h"
#include "World/World.h"
#include "World/Entity/EventManagerComponent.h"
#include "World/Entity/GroupComponent.h"

namespace traktor::theater
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.theater.TheaterEntityComponent", TheaterEntityComponent, world::IEntityComponent)

TheaterEntityComponent::TheaterEntityComponent(Performance* performance)
:	m_performance(performance)
{
}

void TheaterEntityComponent::destroy()
{
	m_owner = nullptr;
	m_performance = nullptr;
}

void TheaterEntityComponent::setOwner(world::Entity* owner)
{
	m_owner = owner;
}

void TheaterEntityComponent::setTransform(const Transform& transform)
{
	// Nothing to do; tracks are relative the owner's transform which
	// is read when the performance is evaluated.
}

Aabb3 TheaterEntityComponent::getBoundingBox() const
{
	return Aabb3();
}

bool TheaterEntityComponent::allowConcurrentUpdate() const
{
	// This component modify entities other than its owner, thus cannot
	// be updated concurrently.
	return false;
}

void TheaterEntityComponent::update(const world::UpdateParams& update)
{
	if (m_owner == nullptr)
		return;

	const world::GroupComponent* group = m_owner->getComponent< world::GroupComponent >();
	if (group == nullptr)
		return;

	world::World* world = m_owner->getWorld();

	const GroupEntityResolver resolver(group);
	m_performance->update(
		resolver,
		(world != nullptr) ? world->getComponent< world::EventManagerComponent >() : nullptr,
		m_owner,
		m_owner->getTransform(),
		update.totalTime,
		update.deltaTime
	);
}

bool TheaterEntityComponent::play(const std::wstring& actName)
{
	return m_performance->play(actName);
}

void TheaterEntityComponent::stop()
{
	m_performance->stop();
}

bool TheaterEntityComponent::isPlaying() const
{
	return m_performance->isPlaying();
}

}
