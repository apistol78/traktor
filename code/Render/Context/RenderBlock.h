/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_RenderBlock_H
#define traktor_render_RenderBlock_H

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
#if defined(_DEBUG)
	const char* name;
#endif
	float distance;
	Ref< IProgram > program;
	ProgramParameters* programParams;

	RenderBlock()
#if defined(_DEBUG)
	:	name(0)
	,	distance(0.0f)
#else
	:	distance(0.0f)
#endif
	,	program(0)
	,	programParams(0)
	{
	}

	virtual ~RenderBlock() {}

	virtual void render(IRenderView* renderView, const ProgramParameters* globalParameters) const = 0;
};

/*! \brief Null render block.
 * \ingroup Render
 *
 * Doesn't render anything; just update
 * shader parameters if available.
 */
class T_DLLCLASS NullRenderBlock : public RenderBlock
{
public:
	virtual void render(IRenderView* renderView, const ProgramParameters* globalParameters) const T_OVERRIDE T_FINAL;
};

/*! \brief Simple render block.
 * \ingroup Render
 */
class T_DLLCLASS SimpleRenderBlock : public RenderBlock
{
public:
	Ref< IndexBuffer > indexBuffer;
	Ref< VertexBuffer > vertexBuffer;
	Primitives primitives;

	virtual void render(IRenderView* renderView, const ProgramParameters* globalParameters) const T_OVERRIDE T_FINAL;
};

/*! \brief Instancing render block.
 * \ingroup Render
 */
class T_DLLCLASS InstancingRenderBlock : public RenderBlock
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

	virtual void render(IRenderView* renderView, const ProgramParameters* globalParameters) const T_OVERRIDE T_FINAL;
};

/*! \brief Instancing render block.
 * \ingroup Render
 */
class T_DLLCLASS IndexedInstancingRenderBlock : public RenderBlock
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
	:	indexBuffer(0)
	,	vertexBuffer(0)
	,	primitive(PtPoints)
	,	offset(0)
	,	count(0)
	,	minIndex(0)
	,	maxIndex(0)
	,	instanceCount(0)
	{
	}

	virtual void render(IRenderView* renderView, const ProgramParameters* globalParameters) const T_OVERRIDE T_FINAL;
};

/*! \brief Non-indexed primitives render block.
 * \ingroup Render
 */
class T_DLLCLASS NonIndexedRenderBlock : public RenderBlock
{
public:
	Ref< VertexBuffer > vertexBuffer;
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

	virtual void render(IRenderView* renderView, const ProgramParameters* globalParameters) const T_OVERRIDE T_FINAL;
};

/*! \brief Indexed primitives render block.
 * \ingroup Render
 */
class T_DLLCLASS IndexedRenderBlock : public RenderBlock
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
	:	indexBuffer(0)
	,	vertexBuffer(0)
	,	primitive(PtPoints)
	,	offset(0)
	,	count(0)
	,	minIndex(0)
	,	maxIndex(0)
	{
	}

	virtual void render(IRenderView* renderView, const ProgramParameters* globalParameters) const T_OVERRIDE T_FINAL;
};

/*! \brief Begin target render block.
 * \ingroup Render
 */
class T_DLLCLASS TargetBeginRenderBlock : public RenderBlock
{
public:
	Ref< RenderTargetSet > renderTargetSet;
	int32_t renderTargetIndex;

	TargetBeginRenderBlock()
	:	RenderBlock()
	,	renderTargetSet(0)
	,	renderTargetIndex(0)
	{
	}

	virtual void render(IRenderView* renderView, const ProgramParameters* globalParameters) const T_OVERRIDE T_FINAL;
};

/*! \brief End target render block.
 * \ingroup Render
 */
class T_DLLCLASS TargetEndRenderBlock : public RenderBlock
{
public:
	virtual void render(IRenderView* renderView, const ProgramParameters* globalParameters) const T_OVERRIDE T_FINAL;
};

/*! \brief Clear target render block.
 * \ingroup Render
 */
class T_DLLCLASS TargetClearRenderBlock : public RenderBlock
{
public:
	uint32_t clearMask;
	Color4f clearColor;
	float clearDepth;
	uint8_t clearStencil;

	TargetClearRenderBlock()
	:	RenderBlock()
	,	clearMask(0)
	,	clearColor(0.0f, 0.0f, 0.0f, 0.0f)
	,	clearDepth(1.0f)
	,	clearStencil(0)
	{
	}

	virtual void render(IRenderView* renderView, const ProgramParameters* globalParameters) const T_OVERRIDE T_FINAL;
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

	virtual void render(IRenderView* renderView, const ProgramParameters* globalParameters) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_render_RenderBlock_H
