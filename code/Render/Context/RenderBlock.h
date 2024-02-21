/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <functional>
#include "Core/Config.h"
#include "Core/Math/Color4f.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class IBufferView;
class IProgram;
class IRenderTargetSet;
class IRenderView;
class ITexture;
class IVertexLayout;
class ProgramParameters;

/*! Render block base class.
 * \ingroup Render
 */
class T_DLLCLASS RenderBlock
{
public:
	std::wstring name;

	virtual ~RenderBlock() {}

	virtual void render(IRenderView* renderView) const = 0;
};

/*! Null render block.
 * \ingroup Render
 *
 * Doesn't render anything; just set
 * program's parameters.
 */
class T_DLLCLASS NullRenderBlock : public RenderBlock
{
public:
	IProgram* program = nullptr;
	ProgramParameters* programParams = nullptr;
	
	virtual void render(IRenderView* renderView) const override final;
};

/*! Compute render block.
 * \ingroup Render
 */
class T_DLLCLASS ComputeRenderBlock : public RenderBlock
{
public:
	IProgram* program = nullptr;
	ProgramParameters* programParams = nullptr;
	int32_t workSize[3] = { 1, 1, 1 };

	virtual void render(IRenderView* renderView) const override final;
};

/*! Drawable render block base class.
 * \ingroup Render
 *
 * A "drawable" block needs to be sortable
 * thus public access to sorting keys.
 */
class T_DLLCLASS DrawableRenderBlock : public RenderBlock
{
public:
	float distance = 0.0f;
	IProgram* program = nullptr;
	ProgramParameters* programParams = nullptr;
};

/*! Simple render block.
 * \ingroup Render
 */
class T_DLLCLASS SimpleRenderBlock : public DrawableRenderBlock
{
public:
	const IBufferView* indexBuffer = nullptr;
	IndexType indexType = IndexType::Void;
	const IBufferView* vertexBuffer = nullptr;
	const IVertexLayout* vertexLayout = nullptr;
	Primitives primitives;

	virtual void render(IRenderView* renderView) const override final;
};

/*! Instancing render block.
 * \ingroup Render
 */
class T_DLLCLASS InstancingRenderBlock : public DrawableRenderBlock
{
public:
	const IBufferView* indexBuffer = nullptr;
	IndexType indexType = IndexType::Void;
	const IBufferView* vertexBuffer = nullptr;
	const IVertexLayout* vertexLayout = nullptr;
	Primitives primitives;
	uint32_t count = 0;

	virtual void render(IRenderView* renderView) const override final;
};

/*! Instancing render block.
 * \ingroup Render
 */
class T_DLLCLASS IndexedInstancingRenderBlock : public DrawableRenderBlock
{
public:
	const IBufferView* indexBuffer = nullptr;
	IndexType indexType = IndexType::Void;
	const IBufferView* vertexBuffer = nullptr;
	const IVertexLayout* vertexLayout = nullptr;
	PrimitiveType primitive = PrimitiveType::Points;
	uint32_t offset = 0;
	uint32_t count = 0;
	uint32_t minIndex = 0;
	uint32_t maxIndex = 0;
	uint32_t instanceCount = 0;

	virtual void render(IRenderView* renderView) const override final;
};

/*! Non-indexed primitives render block.
 * \ingroup Render
 */
class T_DLLCLASS NonIndexedRenderBlock : public DrawableRenderBlock
{
public:
	const IBufferView* vertexBuffer = nullptr;
	const IVertexLayout* vertexLayout = nullptr;
	PrimitiveType primitive = PrimitiveType::Points;
	uint32_t offset = 0;
	uint32_t count = 0;

	virtual void render(IRenderView* renderView) const override final;
};

/*! Indexed primitives render block.
 * \ingroup Render
 */
class T_DLLCLASS IndexedRenderBlock : public DrawableRenderBlock
{
public:
	const IBufferView* indexBuffer = nullptr;
	IndexType indexType = IndexType::Void;
	const IBufferView* vertexBuffer = nullptr;
	const IVertexLayout* vertexLayout = nullptr;
	PrimitiveType primitive = PrimitiveType::Points;
	uint32_t offset = 0;
	uint32_t count = 0;
	uint32_t minIndex = 0;
	uint32_t maxIndex = 0;

	virtual void render(IRenderView* renderView) const override final;
};

/*! Indirect render block.
 * \ingroup Render
 */
class T_DLLCLASS IndirectRenderBlock : public DrawableRenderBlock
{
public:
	const IBufferView* indexBuffer = nullptr;
	IndexType indexType = IndexType::Void;
	const IBufferView* vertexBuffer = nullptr;
	const IVertexLayout* vertexLayout = nullptr;
	PrimitiveType primitive = PrimitiveType::Points;
	const IBufferView* drawBuffer = nullptr;
	uint32_t drawCount = 0;

	virtual void render(IRenderView* renderView) const override final;
};

/*! Begin pass render block.
 * \ingroup Render
 */
class T_DLLCLASS BeginPassRenderBlock : public RenderBlock
{
public:
	IRenderTargetSet* renderTargetSet = nullptr;
	int32_t renderTargetIndex = -1;
	Clear clear;
	uint32_t load = 0;
	uint32_t store = 0;

	virtual void render(IRenderView* renderView) const override final;
};

/*! End pass render block.
 * \ingroup Render
 */
class T_DLLCLASS EndPassRenderBlock : public RenderBlock
{
public:
	virtual void render(IRenderView* renderView) const override final;
};

/*! Present backbuffer.
 * \ingroup Render
 */
class T_DLLCLASS PresentRenderBlock : public RenderBlock
{
public:
	virtual void render(IRenderView* renderView) const override final;
};

/*! Set target viewport block.
 * \ingroup Render
 */
class T_DLLCLASS SetViewportRenderBlock : public RenderBlock
{
public:
	Viewport viewport;

	virtual void render(IRenderView* renderView) const override final;
};

/*! Barrier block.
 * \ingroup Render
 */
class T_DLLCLASS BarrierRenderBlock : public RenderBlock
{
public:
	Stage from = Stage::Invalid;
	Stage to = Stage::Invalid;
	ITexture* written = nullptr;
	uint32_t writtenMip = 0;

	BarrierRenderBlock() = default;

	explicit BarrierRenderBlock(Stage from_, Stage to_, ITexture* written_, uint32_t writtenMip_)
	:	from(from_)
	,	to(to_)
	,	written(written_)
	,	writtenMip(writtenMip_)
	{
	}

	virtual void render(IRenderView* renderView) const override final;
};

/*! Lambda callback render block.
 * \ingroup Render
 */
class T_DLLCLASS LambdaRenderBlock : public RenderBlock
{
public:
	std::function< void(IRenderView*) > lambda;

	virtual void render(IRenderView* renderView) const override final;
};

/*! Time query begin render block. */
class T_DLLCLASS ProfileBeginRenderBlock : public RenderBlock
{
public:
	int32_t* queryHandle = nullptr;

	virtual void render(IRenderView* renderView) const override final;
};

/*! Time query end render block. */
class T_DLLCLASS ProfileEndRenderBlock : public RenderBlock
{
public:
	int32_t* queryHandle = nullptr;

	virtual void render(IRenderView* renderView) const override final;
};

/*! Time query report render block. */
class T_DLLCLASS ProfileReportRenderBlock : public RenderBlock
{
public:
	typedef std::function< void(const std::wstring&, double, double) > fn_sink_t;

	std::wstring name;
	int32_t* queryHandle = nullptr;
	int32_t* referenceQueryHandle = nullptr;
	double offset;
	fn_sink_t sink;

	virtual void render(IRenderView* renderView) const override final;
};

}
