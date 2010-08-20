#ifndef traktor_render_RenderBlock_H
#define traktor_render_RenderBlock_H

#include "Core/Config.h"
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

/*! \group Render block debug name.
 * \ingroup Render
 * \{
 */

#if defined(T_USE_RENDER_MARKERS)
#	define T_USE_RENDERBLOCK_NAME
#endif

/*! \} */

class IndexBuffer;
class IProgram;
class IRenderView;
class ProgramParameters;
class RenderTargetSet;
class VertexBuffer;

/*! \brief Render block base class.
 * \ingroup Render
 */
class T_DLLCLASS RenderBlock
{
public:
#if defined(T_USE_RENDERBLOCK_NAME)
	const char* name;
#endif
	float distance;
	IProgram* program;
	ProgramParameters* programParams;

	RenderBlock()
#if defined(T_USE_RENDERBLOCK_NAME)
	:	name(0)
	,	distance(0.0f)
#else
	:	distance(0.0f)
#endif
	,	program(0)
	,	programParams(0)
	{
	}

	virtual void render(IRenderView* renderView) const = 0;
};

#pragma warning( push )
#pragma warning( disable : 4311 )	// Pointer truncation

/*! \brief Null render block.
 * \ingroup Render
 *
 * Doesn't render anything; just update
 * shader parameters if available.
 */
class T_DLLCLASS NullRenderBlock : public RenderBlock
{
public:
	virtual void render(IRenderView* renderView) const;
};

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

/*! \brief Begin target render block.
 * \ingroup Render
 */
class T_DLLCLASS TargetBeginRenderBlock : public RenderBlock
{
public:
	RenderTargetSet* renderTargetSet;
	int32_t renderTargetIndex;
	bool keepDepthStencil;

	TargetBeginRenderBlock()
	:	RenderBlock()
	,	renderTargetSet(0)
	,	renderTargetIndex(0)
	,	keepDepthStencil(false)
	{
	}

	virtual void render(IRenderView* renderView) const;
};

/*! \brief End target render block.
 * \ingroup Render
 */
class T_DLLCLASS TargetEndRenderBlock : public RenderBlock
{
public:
	virtual void render(IRenderView* renderView) const;
};

/*! \brief Clear target render block.
 * \ingroup Render
 */
class T_DLLCLASS TargetClearRenderBlock : public RenderBlock
{
public:
	uint32_t clearMask;
	float clearColor[4];
	float clearDepth;
	uint8_t clearStencil;

	TargetClearRenderBlock()
	:	RenderBlock()
	,	clearMask(0)
	,	clearDepth(1.0f)
	,	clearStencil(0)
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
