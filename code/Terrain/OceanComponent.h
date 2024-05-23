/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Color4f.h"
#include "Core/Math/Vector4.h"
#include "Render/Types.h"
#include "Resource/Proxy.h"
#include "World/IEntityComponent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class Buffer;
class IRenderSystem;
class ITexture;
class IVertexLayout;
class RenderContext;
class Shader;

}

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::world
{

class IWorldRenderPass;
class WorldRenderView;
class WorldSetupContext;

}

namespace traktor::terrain
{

class OceanComponentData;
class Terrain;

/*! Ocean component.
 * \ingroup Terrain
 */
class T_DLLCLASS OceanComponent : public world::IEntityComponent
{
	T_RTTI_CLASS;

public:
	static constexpr int32_t MaxWaves = 32;

	virtual ~OceanComponent();

	bool create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, const OceanComponentData& data);

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

	void setup(
		const world::WorldSetupContext& context,
		const world::WorldRenderView& worldRenderView
	);

	void build(
		render::RenderContext* renderContext,
		const world::WorldRenderView& worldRenderView,
		const world::IWorldRenderPass& worldRenderPass,
		bool reflectionEnable
	);

	void setShallowTint(const Color4f& shallowTint) { m_shallowTint = shallowTint; }

	const Color4f& getShallowTint() const { return m_shallowTint; }

	void setDeepColor(const Color4f& deepColor) { m_deepColor = deepColor; }

	const Color4f& getDeepColor() const { return m_deepColor; }

	void setOpacity(float opacity) { m_opacity = opacity; }

	float getOpacity() const { return m_opacity; }

private:
	world::Entity* m_owner = nullptr;
	resource::Proxy< Terrain > m_terrain;
	resource::Proxy< render::Shader > m_shaderWave;	//!< Compute shader to generate wave maps.
	resource::Proxy< render::Shader > m_shader;

	Ref< render::ITexture > m_spectrumTexture;
	Ref< render::ITexture > m_evolvedSpectrumTextures[2];
	Ref< render::ITexture > m_foamTexture;

	Ref< const render::IVertexLayout > m_vertexLayout;
	Ref< render::Buffer > m_indexBuffer;
	Ref< render::Buffer > m_vertexBuffer;
	render::Primitives m_primitives;
	Color4f m_shallowTint;
	Color4f m_deepColor;
	float m_opacity = 0.5f;
	float m_elevation = 0.0f;

	bool m_spectrumDirty = true;
};

}
