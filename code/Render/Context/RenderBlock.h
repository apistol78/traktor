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

namespace traktor
{
	namespace render
	{

class IndexBuffer;
class IProgram;
class IRenderTargetSet;
class IRenderView;
class ProgramParameters;
class VertexBuffer;

/*! Render block base class.
 * \ingroup Render
 */
class T_DLLCLASS RenderBlock
{
public:
#if defined(_DEBUG)
	const char* name;
#endif
	float distance;
	Ref< IProgram > program;
	ProgramParameters* programParams;

	RenderBlock()
#if defined(_DEBUG)
	:	name(nullptr)
	,	distance(0.0f)
#else
	:	distance(0.0f)
#endif
	,	programParams(nullptr)
	{
	}

	virtual ~RenderBlock() {}

	virtual void render(IRenderView* renderView) const = 0;
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
	float distance;
	Ref< IProgram > program;
	ProgramParameters* programParams;

	DrawableRenderBlock()
	:	distance(0.0f)
	,	programParams(nullptr)
	{
	}
};

/*! Null render block.
 * \ingroup Render
 *
 * Doesn't render anything; just update
 * shader parameters if available.
 */
class T_DLLCLASS NullRenderBlock : public DrawableRenderBlock
{
public:
	virtual void render(IRenderView* renderView) const override final;
};

/*! Simple render block.
 * \ingroup Render
 */
class T_DLLCLASS SimpleRenderBlock : public DrawableRenderBlock
{
public:
	Ref< IndexBuffer > indexBuffer;
	Ref< VertexBuffer > vertexBuffer;
	Primitives primitives;

	virtual void render(IRenderView* renderView) const override final;
};

/*! Instancing render block.
 * \ingroup Render
 */
class T_DLLCLASS InstancingRenderBlock : public DrawableRenderBlock
{
public:
	Ref< IndexBuffer > indexBuffer;
	Ref< VertexBuffer > vertexBuffer;
	Primitives primitives;
	uint32_t count;

	InstancingRenderBlock()
	:	count(0)
	{
	}

	virtual void render(IRenderView* renderView) const override final;
};

/*! Instancing render block.
 * \ingroup Render
 */
class T_DLLCLASS IndexedInstancingRenderBlock : public DrawableRenderBlock
{
public:
	Ref< IndexBuffer > indexBuffer;
	Ref< VertexBuffer > vertexBuffer;
	PrimitiveType primitive;
	uint32_t offset;
	uint32_t count;
	uint32_t minIndex;
	uint32_t maxIndex;
	uint32_t instanceCount;

	IndexedInstancingRenderBlock()
	:	primitive(PtPoints)
	,	offset(0)
	,	count(0)
	,	minIndex(0)
	,	maxIndex(0)
	,	instanceCount(0)
	{
	}

	virtual void render(IRenderView* renderView) const override final;
};

/*! Non-indexed primitives render block.
 * \ingroup Render
 */
class T_DLLCLASS NonIndexedRenderBlock : public DrawableRenderBlock
{
public:
	Ref< VertexBuffer > vertexBuffer;
	PrimitiveType primitive;
	uint32_t offset;
	uint32_t count;

	NonIndexedRenderBlock()
	:	primitive(PtPoints)
	,	offset(0)
	,	count(0)
	{
	}

	virtual void render(IRenderView* renderView) const override final;
};

/*! Indexed primitives render block.
 * \ingroup Render
 */
class T_DLLCLASS IndexedRenderBlock : public DrawableRenderBlock
{
public:
	Ref< IndexBuffer > indexBuffer;
	Ref< VertexBuffer > vertexBuffer;
	PrimitiveType primitive;
	uint32_t offset;
	uint32_t count;
	uint32_t minIndex;
	uint32_t maxIndex;

	IndexedRenderBlock()
	:	primitive(PtPoints)
	,	offset(0)
	,	count(0)
	,	minIndex(0)
	,	maxIndex(0)
	{
	}

	virtual void render(IRenderView* renderView) const override final;
};

/*! Begin target render block.
 * \ingroup Render
 */
class T_DLLCLASS TargetBeginRenderBlock : public RenderBlock
{
public:
	Ref< IRenderTargetSet > renderTargetSet;
	int32_t renderTargetIndex;
	Clear clear;

	TargetBeginRenderBlock()
	:	RenderBlock()
	,	renderTargetIndex(-1)
	{
		clear.mask = 0;
	}

	virtual void render(IRenderView* renderView) const override final;
};

/*! End target render block.
 * \ingroup Render
 */
class T_DLLCLASS TargetEndRenderBlock : public RenderBlock
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

/*! Lambda callback render block.
 * \ingroup Render
 */
class T_DLLCLASS LambdaRenderBlock : public RenderBlock
{
public:
	std::function< void(IRenderView*) > lambda;

	virtual void render(IRenderView* renderView) const override final;
};

	}
}

