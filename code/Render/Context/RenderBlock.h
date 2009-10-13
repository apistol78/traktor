#ifndef traktor_render_RenderBlock_H
#define traktor_render_RenderBlock_H

#include "Core/Config.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class IRenderView;
class RenderTargetSet;
class Shader;
class ShaderParameters;
class IndexBuffer;
class VertexBuffer;

/*! \brief Type of render block.
 * \ingroup Render
 */
enum RenderBlockType
{
	RbtOpaque,
	RbtAlphaBlend
};

/*! \brief Render block base class.
 * \ingroup Render
 */
class T_DLLCLASS RenderBlock
{
public:
	uint8_t type;
	float distance;
	Shader* shader;
	ShaderParameters* shaderParams;

	RenderBlock()
	:	type(RbtOpaque)
	,	distance(0.0f)
	,	shader(0)
	,	shaderParams(0)
	{
	}

	virtual void render(IRenderView* renderView) const = 0;
};

#pragma warning( push )
#pragma warning( disable : 4311 )	// Pointer truncation

/*! \brief Simple render block.
 * \ingroup Render
 */
class T_DLLCLASS SimpleRenderBlock : public RenderBlock
{
public:
	IndexBuffer* indexBuffer;
	VertexBuffer* vertexBuffer;
	const Primitives* primitives;

	SimpleRenderBlock()
	:	indexBuffer(0)
	,	vertexBuffer(0)
	,	primitives(0)
	{
	}

	virtual void render(IRenderView* renderView) const;
};

/*! \brief Non-indexed primitives render block.
 * \ingroup Render
 */
class T_DLLCLASS NonIndexedRenderBlock : public RenderBlock
{
public:
	VertexBuffer* vertexBuffer;
	PrimitiveType primitive;
	uint32_t offset;
	uint32_t count;

	NonIndexedRenderBlock()
	:	vertexBuffer(0)
	,	primitive(PtPoints)
	,	offset(0)
	,	count(0)
	{
	}

	virtual void render(IRenderView* renderView) const;
};

/*! \brief Indexed primitives render block.
 * \ingroup Render
 */
class T_DLLCLASS IndexedRenderBlock : public RenderBlock
{
public:
	IndexBuffer* indexBuffer;
	VertexBuffer* vertexBuffer;
	PrimitiveType primitive;
	uint32_t offset;
	uint32_t count;
	uint32_t minIndex;
	uint32_t maxIndex;

	IndexedRenderBlock()
	:	indexBuffer(0)
	,	vertexBuffer(0)
	,	primitive(PtPoints)
	,	offset(0)
	,	count(0)
	,	minIndex(0)
	,	maxIndex(0)
	{
	}

	virtual void render(IRenderView* renderView) const;
};

/*! \brief Alternative target render block.
 * \ingroup Render
 */
class T_DLLCLASS TargetRenderBlock : public RenderBlock
{
public:
	RenderTargetSet* renderTargetSet;
	int32_t renderTargetIndex;
	bool keepDepthStencil;
	uint32_t clearMask;
	float clearColor[4];
	float clearDepth;
	uint8_t clearStencil;
	RenderBlock* inner;

	TargetRenderBlock()
	:	RenderBlock()
	,	renderTargetSet(0)
	,	renderTargetIndex(0)
	,	keepDepthStencil(false)
	,	clearMask(0)
	,	clearDepth(1.0f)
	,	clearStencil(0)
	,	inner(0)
	{
		clearColor[0] =
		clearColor[1] =
		clearColor[2] =
		clearColor[3] = 0.0f;
	}

	virtual void render(IRenderView* renderView) const;
};

/*! \brief Chain render block.
 * \ingroup Render
 */
class T_DLLCLASS ChainRenderBlock : public RenderBlock
{
public:
	RenderBlock* inner;
	RenderBlock* next;

	ChainRenderBlock()
	:	RenderBlock()
	,	inner(0)
	,	next(0)
	{
	}

	virtual void render(IRenderView* renderView) const;
};

#pragma warning( pop )

	}
}

#endif	// traktor_render_RenderBlock_H
