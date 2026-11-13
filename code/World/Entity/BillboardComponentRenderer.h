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
#include "Core/Containers/SmallMap.h"
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
class IBufferView;
class IRenderSystem;
class ITexture;
class IVertexLayout;
class Shader;

}

namespace traktor::world
{

/*! Billboard entity renderer.
 * \ingroup World
 *
 * Quads are built once per view in setup so every pass, shadows included, draw the same quads;
 * billboards sharing shader and atlas are drawn with a single call.
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
	/*! A run of quads sharing shader and atlas, drawn as one call. */
	struct Batch
	{
		const render::Shader* shader;
		render::ITexture* albedoTexture;
		render::ITexture* normalTexture;
		uint32_t offset; //!< First index.
		uint32_t count;  //!< Number of triangles.
	};

	/*! Quads of a view, valid until the view is setup again. */
	struct ViewState
	{
		const render::IBufferView* vertexBufferView = nullptr;
		AlignedVector< Batch > batches;
	};

	Ref< const render::IVertexLayout > m_vertexLayout;
	Ref< render::Buffer > m_vertexBuffer;
	Ref< render::Buffer > m_indexBuffer;
	SmallMap< int32_t, ViewState > m_viewStates;

	bool initialize(render::IRenderSystem* renderSystem);
};

}
