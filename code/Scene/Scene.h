/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

struct UpdateParams;

class World;
class WorldRenderSettings;

}

namespace traktor::scene
{

/*! Scene
 * \ingroup Scene
 *
 * A scene holds information about entities,
 * world renderer configuration and post processing.
 */
class T_DLLCLASS Scene : public Object
{
	T_RTTI_CLASS;

public:
	explicit Scene(
		const world::WorldRenderSettings* worldRenderSettings,
		world::World* world
	);

	explicit Scene(Scene* scene);

	virtual ~Scene();

	void destroy();

	void update(const world::UpdateParams& update);

	const world::WorldRenderSettings* getWorldRenderSettings() const;

	world::World* getWorld() const;

private:
	Ref< const world::WorldRenderSettings > m_worldRenderSettings;
	Ref< world::World > m_world;
};

}
