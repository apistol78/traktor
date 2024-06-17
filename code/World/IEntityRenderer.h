/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ObjectStore;

}

namespace traktor::world
{

class IWorldRenderPass;
class WorldBuildContext;
class WorldRenderView;
class WorldSetupContext;

/*! Entity renderer.
 * \ingroup World
 *
 * Each renderable type should have
 * a matching EntityRenderer.
 */
class T_DLLCLASS IEntityRenderer : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool initialize(const ObjectStore& objectStore) = 0;

	virtual const TypeInfoSet getRenderableTypes() const = 0;

	/*! Setup pass. */
	virtual void setup(
		const WorldSetupContext& context,
		const WorldRenderView& worldRenderView,
		Object* renderable
	) = 0;

	/*! Setup pass. */
	virtual void setup(
		const WorldSetupContext& context
	) = 0;

	/*! Build pass. */
	virtual void build(
		const WorldBuildContext& context,
		const WorldRenderView& worldRenderView,
		const IWorldRenderPass& worldRenderPass,
		Object* renderable
	) = 0;

	/*! Build pass.
	 *
	 * Flush whatever queues that the entity
	 * renderer might have used.
	 */
	virtual void build(
		const WorldBuildContext& context,
		const WorldRenderView& worldRenderView,
		const IWorldRenderPass& worldRenderPass
	) = 0;
};

}
