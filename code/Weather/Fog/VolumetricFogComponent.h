/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Resource/Proxy.h"
#include "World/IEntityComponent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WEATHER_EXPORT)
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
class WorldBuildContext;
class WorldRenderView;
class WorldSetupContext;

}

namespace traktor::weather
{

/*!
 */
class T_DLLCLASS VolumetricFogComponent : public world::IEntityComponent
{
	T_RTTI_CLASS;

public:
	explicit VolumetricFogComponent(const resource::Proxy< render::Shader >& shader);

	bool create(render::IRenderSystem* renderSystem);

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

	void setup(const world::WorldSetupContext& context, const world::WorldRenderView& worldRenderView);

	void build(const world::WorldBuildContext& context, const world::WorldRenderView& worldRenderView, const world::IWorldRenderPass& worldRenderPass);

private:
	world::Entity* m_owner = nullptr;
	resource::Proxy< render::Shader > m_shader;
	Ref< render::ITexture > m_fogVolumeTexture;
};

}
