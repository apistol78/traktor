/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "World/IEntityRenderer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

/*! Compute texture entity renderer.
 * \ingroup World
 *
 * Hooks ComputeTextureComponent into the render graph by
 * dispatching compute passes during world setup.
 */
class T_DLLCLASS ComputeTextureRenderer : public IEntityRenderer
{
	T_RTTI_CLASS;

public:
	virtual bool initialize(const ObjectStore& objectStore) override final;

	virtual const TypeInfoSet getRenderableTypes() const override final;

	virtual void setup(
		const WorldSetupContext& context,
		const WorldRenderView& worldRenderView,
		Object* renderable
	) override final;

	virtual void setup(
		const WorldSetupContext& context
	) override final;

	virtual void build(
		const WorldBuildContext& context,
		const WorldRenderView& worldRenderView,
		const IWorldRenderPass& worldRenderPass,
		Object* renderable
	) override final;

	virtual void build(
		const WorldBuildContext& context,
		const WorldRenderView& worldRenderView,
		const IWorldRenderPass& worldRenderPass
	) override final;
};

}
