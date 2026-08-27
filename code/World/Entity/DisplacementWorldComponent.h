/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Vector4.h"
#include "Core/Ref.h"
#include "Render/Frame/RenderGraphTypes.h"
#include "Render/Shader.h"
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

class Buffer;
class IRenderSystem;
class IRenderTargetSet;
class ITexture;
class IVertexLayout;
class ProgramParameters;
class RenderGraph;
class ScreenRenderer;

}

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::world
{

class DisplacementEntityComponent;
class DisplacementWorldComponentData;
class WorldRenderView;

/*! Displacement world component.
 * \ingroup World
 */
class T_DLLCLASS DisplacementWorldComponent : public IWorldComponent
{
	T_RTTI_CLASS;

public:
	bool create(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		const DisplacementWorldComponentData& data);

	virtual void destroy() override final;

	virtual void update(World* world, const UpdateParams& update) override final;

	void setup(
		render::RenderGraph& renderGraph,
		const WorldRenderView& worldRenderView,
		const AlignedVector< Object* >& renderables);

	render::ITexture* getMask() const;

	const Vector4& getMaskExtent() const { return m_maskExtent; }

	static void getPermutation(const DisplacementWorldComponent* displacement, const render::Shader* shader, render::Shader::Permutation& inoutPerm);

	static void setSharedParameters(const DisplacementWorldComponent* displacement, render::ProgramParameters* programParams);

private:
#pragma pack(1)

	struct VolumeData
	{
		float centerX;
		float centerZ;
		float radius;
		float strength;
		float falloff;
		float press;
		float dummy0;
		float dummy1;
	};

#pragma pack()

	Ref< render::IRenderSystem > m_renderSystem;
	Ref< render::ScreenRenderer > m_screenRenderer;
	resource::Proxy< render::Shader > m_reprojectShader;
	resource::Proxy< render::Shader > m_rasterizeVolumesShader;
	Ref< render::IRenderTargetSet > m_maskTargetSet[2];
	Ref< const render::IVertexLayout > m_vertexLayout;
	Ref< render::Buffer > m_vertexBuffer;
	Ref< render::Buffer > m_indexBuffer;
	Ref< render::Buffer > m_volumeBuffer;
	Vector4 m_maskExtent = Vector4::zero();
	Vector4 m_center = Vector4::origo();
	int32_t m_resolution = 0;
	float m_extent = 0.0f;
	float m_fadeRate = 0.0f;
	int32_t m_current = 0;
	bool m_haveMask = false;
};

}
