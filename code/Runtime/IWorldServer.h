/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Runtime/IServer.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

class IEntityFactory;
class IEntityRenderer;
class IWorldRenderer;
class WorldEntityRenderers;
class WorldRenderSettings;

}

namespace traktor::runtime
{

/*! World server.
 * \ingroup Runtime
 *
 * "World.ShadowQuality" - Shadow filter quality.
 */
class T_DLLCLASS IWorldServer : public IServer
{
	T_RTTI_CLASS;

public:
	virtual void addEntityFactory(world::IEntityFactory* entityFactory) = 0;

	virtual void removeEntityFactory(world::IEntityFactory* entityFactory) = 0;

	virtual void addEntityRenderer(world::IEntityRenderer* entityRenderer) = 0;

	virtual void removeEntityRenderer(world::IEntityRenderer* entityRenderer) = 0;

	virtual world::IEntityFactory* getEntityFactory() = 0;

	virtual world::WorldEntityRenderers* getEntityRenderers() = 0;

	virtual Ref< world::IWorldRenderer > createWorldRenderer(const world::WorldRenderSettings* worldRenderSettings) = 0;
};

}
