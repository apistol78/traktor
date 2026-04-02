/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/Math/Color4f.h"
#include "Resource/Proxy.h"
#include "World/IWorldComponent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class IRenderSystem;
class ITexture;
class ProgramParameters;
class Shader;

}

namespace traktor::world
{

class IWorldRenderPass;
class FogComponentData;
class WorldBuildContext;
class WorldRenderView;
class WorldSetupContext;

/*!
 */
class T_DLLCLASS FogComponent : public IWorldComponent
{
	T_RTTI_CLASS;

public:
	explicit FogComponent(const FogComponentData* data, const resource::Proxy< render::Shader >& shader);

	bool create(render::IRenderSystem* renderSystem);

	virtual void destroy() override final;

	virtual void update(World* world, const UpdateParams& update) override final;

	void build(const WorldBuildContext& context, const WorldRenderView& worldRenderView, const IWorldRenderPass& worldRenderPass);

	static void setupSharedParameters(const FogComponent* fog, float viewNearZ, float viewFarZ, render::ProgramParameters* parameters);

	render::ITexture* getFogVolumeTexture() const { return m_fogVolumeTexture; }

	float getMaxDistance() const { return m_maxDistance; }

	float getMaxScattering() const { return m_maxScattering; }

	int32_t getSliceCount() const { return m_sliceCount; }

private:
	friend class WorldRendererDeferred;
	friend class WorldRendererForward;

	// Distance fog.
	float m_fogDistance = 90.0f;
	float m_fogElevation = 0.0f;
	float m_fogDensity = 0.0f;
	float m_fogDensityMax = 1.0f;
	Color4f m_fogColor = Color4f(1.0f, 1.0f, 1.0f, 1.0f);

	// Volumetric fog.
	resource::Proxy< render::Shader > m_shader;
	Ref< render::ITexture > m_fogVolumeTexture;
	bool m_volumetricFogEnable = false;
	float m_maxDistance = 0.0f;
	float m_maxScattering = 0.0f;
	int32_t m_sliceCount = 0;
	Color4f m_mediumColor = Color4f(1.0f, 1.0f, 1.0f, 1.0f);
	float m_mediumDensity = 0.0f;
};

}
