/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Theater/TheaterWorldComponent.h"

#include "Core/Math/Transform.h"
#include "Theater/Performance.h"
#include "Theater/WorldEntityResolver.h"
#include "World/World.h"
#include "World/WorldTypes.h"
#include "World/Entity/EventManagerComponent.h"

namespace traktor::theater
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.theater.TheaterWorldComponent", TheaterWorldComponent, world::IWorldComponent)

TheaterWorldComponent::TheaterWorldComponent(Performance* performance)
:	m_performance(performance)
{
}

void TheaterWorldComponent::destroy()
{
	m_performance = nullptr;
}

void TheaterWorldComponent::update(world::World* world, const world::UpdateParams& update)
{
	const WorldEntityResolver resolver(world);
	m_performance->update(
		resolver,
		world->getComponent< world::EventManagerComponent >(),
		nullptr,
		Transform::identity(),
		update.totalTime,
		update.deltaTime
	);
}

bool TheaterWorldComponent::play(const std::wstring& actName)
{
	return m_performance->play(actName);
}

void TheaterWorldComponent::stop()
{
	m_performance->stop();
}

bool TheaterWorldComponent::isPlaying() const
{
	return m_performance->isPlaying();
}

}
