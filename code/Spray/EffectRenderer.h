/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "World/IEntityRenderer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class IRenderSystem;

}

namespace traktor::spray
{

class MeshRenderer;
class PointRenderer;
class TrailRenderer;

/*! Effect renderer.
 * \ingroup Spray
 */
class T_DLLCLASS EffectRenderer : public world::IEntityRenderer
{
	T_RTTI_CLASS;

public:
	explicit EffectRenderer(render::IRenderSystem* renderSystem, float lod1Distance, float lod2Distance);

	void setLodDistances(float lod1Distance, float lod2Distance);

	virtual bool initialize(const ObjectStore& objectStore) override final;

	virtual const TypeInfoSet getRenderableTypes() const override final;

	virtual void setup(
		const world::WorldSetupContext& context,
		const world::WorldRenderView& worldRenderView,
		Object* renderable
	) override final;

	virtual void setup(
		const world::WorldSetupContext& context
	) override final;

	virtual void build(
		const world::WorldBuildContext& context,
		const world::WorldRenderView& worldRenderView,
		const world::IWorldRenderPass& worldRenderPass,
		Object* renderable
	) override final;

	virtual void build(
		const world::WorldBuildContext& context,
		const world::WorldRenderView& worldRenderView,
		const world::IWorldRenderPass& worldRenderPass
	) override final;

private:
	Ref< PointRenderer > m_pointRenderer;
	Ref< MeshRenderer > m_meshRenderer;
	Ref< TrailRenderer > m_trailRenderer;
};

}
