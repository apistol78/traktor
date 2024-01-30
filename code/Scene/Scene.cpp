/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/SafeDestroy.h"
#include "Scene/Scene.h"
#include "World/Entity.h"
#include "World/World.h"
#include "World/WorldRenderSettings.h"

namespace traktor::scene
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.Scene", Scene, Object)

Scene::Scene(
	const world::WorldRenderSettings* worldRenderSettings,
	world::World* world
)
:	m_worldRenderSettings(worldRenderSettings)
,	m_world(world)
{
}

Scene::Scene(Scene* scene)
:	m_worldRenderSettings(scene->m_worldRenderSettings)
,	m_world(scene->m_world)
{
}

Scene::~Scene()
{
	m_worldRenderSettings = nullptr;
	m_world = nullptr;
}

void Scene::destroy()
{
	m_worldRenderSettings = nullptr;
	safeDestroy(m_world);
}

void Scene::update(const world::UpdateParams& update)
{
	T_FATAL_ASSERT(m_world != nullptr);
	m_world->update(update);
}

const world::WorldRenderSettings* Scene::getWorldRenderSettings() const
{
	return m_worldRenderSettings;
}

world::World* Scene::getWorld() const
{
	return m_world;
}

}
