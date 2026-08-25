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
#include "Core/Ref.h"
#include "World/IEntityRenderer.h"

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
class IVertexLayout;
class Shader;

}

namespace traktor::world
{

/*! Billboard entity renderer.
 * \ingroup World
 *
 * All visible billboards are turned into camera facing quads once per frame, in
 * setup, and written into a single vertex buffer; every pass then draw those quads
 * from the same buffer, batched so that billboards sharing a shader cost one draw
 * call between them.
 *
 * Building the quads in setup rather than in build is what make the passes agree:
 * shadow and depth passes see the very same quads as the visual pass, oriented for
 * the camera rather than for their own view.
 */
class T_DLLCLASS BillboardComponentRenderer : public IEntityRenderer
{
	T_RTTI_CLASS;

public:
	BillboardComponentRenderer() = default;

	explicit BillboardComponentRenderer(render::IRenderSystem* renderSystem);

	virtual bool initialize(const ObjectStore& objectStore) override final;

	virtual const TypeInfoSet getRenderableTypes() const override final;

	virtual void setup(
		const WorldSetupContext& context,
		const WorldRenderView& worldRenderView,
		const AlignedVector< Object* >& renderables) override final;

	virtual void build(
		const WorldBuildContext& context,
		const WorldRenderView& worldRenderView,
		const IWorldRenderPass& worldRenderPass,
		const AlignedVector< Object* >& renderables) override final;

private:
	/*! A run of quads sharing a shader, drawn as one call. */
	struct Batch
	{
		const render::Shader* shader;
		uint32_t offset; //!< First index.
		uint32_t count;  //!< Number of triangles.
	};

	Ref< const render::IVertexLayout > m_vertexLayout;
	Ref< render::Buffer > m_vertexBuffers[4];
	Ref< render::Buffer > m_indexBuffer;
	AlignedVector< Batch > m_batches;
	uint32_t m_cycle = 0;
	uint32_t m_quadCount = 0;

	bool initialize(render::IRenderSystem* renderSystem);
};

}
