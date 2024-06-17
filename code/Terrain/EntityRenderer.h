/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "World/IEntityRenderer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::terrain
{

/*! Terrain entity renderer.
 * \ingroup Terrain
 */
class T_DLLCLASS EntityRenderer : public world::IEntityRenderer
{
	T_RTTI_CLASS;

public:
	explicit EntityRenderer(float terrainDetailDistance, uint32_t terrainCacheSize, bool terrainLayersEnable, bool oceanReflectionEnable);

	void setTerrainDetailDistance(float terrainDetailDistance);

	void setTerrainCacheSize(uint32_t terrainCacheSize);

	void setTerrainLayersEnable(bool terrainLayersEnable);

	void setOceanDynamicReflectionEnable(bool oceanReflectionEnable);

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
	float m_terrainDetailDistance;
	uint32_t m_terrainCacheSize;
	bool m_terrainLayersEnable;
	bool m_oceanReflectionEnable;
};

}
