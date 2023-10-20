/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/Math/Color4f.h"
#include "Resource/Proxy.h"
#include "World/IEntityComponent.h"

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
class Shader;

}

namespace traktor::world
{

class IWorldRenderPass;
class VolumetricFogComponentData;
class WorldBuildContext;
class WorldRenderView;
class WorldSetupContext;

/*!
 */
class T_DLLCLASS VolumetricFogComponent : public IEntityComponent
{
	T_RTTI_CLASS;

public:
	explicit VolumetricFogComponent(const VolumetricFogComponentData* data, const resource::Proxy< render::Shader >& shader);

	bool create(render::IRenderSystem* renderSystem);

	virtual void destroy() override final;

	virtual void setOwner(Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const UpdateParams& update) override final;

	void build(const WorldBuildContext& context, const WorldRenderView& worldRenderView, const IWorldRenderPass& worldRenderPass);

	render::ITexture* getFogVolumeTexture() const { return m_fogVolumeTexture; }

	float getMaxDistance() const { return m_maxDistance; }

	float getMaxScattering() const { return m_maxScattering; }

	int32_t getSliceCount() const { return m_sliceCount; }

private:
	Entity* m_owner = nullptr;
	resource::Proxy< render::Shader > m_shader;
	Ref< render::ITexture > m_fogVolumeTexture;
	float m_maxDistance = 0.0f;
	float m_maxScattering = 0.0f;
	int32_t m_sliceCount = 0;
	int32_t m_sliceCurrent = 0;
	Color4f m_mediumColor = Color4f(1.0f, 1.0f, 1.0f, 1.0f);
	float m_mediumDensity = 0.0f;
};

}
