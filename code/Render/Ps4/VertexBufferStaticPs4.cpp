/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/Ps4/VertexBufferStaticPs4.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VertexBufferStaticPs4", VertexBufferStaticPs4, VertexBufferPs4)

Ref< VertexBufferStaticPs4 > VertexBufferStaticPs4::create(
	ContextPs4* context,
	uint32_t bufferSize,
	const AlignedVector< VertexElement >& vertexElements
)
{
	Ref< VertexBufferStaticPs4 > vb = new VertexBufferStaticPs4(bufferSize);
	vb->m_context = context;
	return vb;
}

VertexBufferStaticPs4::~VertexBufferStaticPs4()
{
	destroy();
}

void VertexBufferStaticPs4::destroy()
{
}

void* VertexBufferStaticPs4::lock()
{
	return 0;
}

void* VertexBufferStaticPs4::lock(uint32_t vertexOffset, uint32_t vertexCount)
{
	T_FATAL_ERROR;
	return 0;
}

void VertexBufferStaticPs4::unlock()
{
	setContentValid(true);
}

void VertexBufferStaticPs4::prepare()
{
	VertexBufferPs4::prepare();
}

VertexBufferStaticPs4::VertexBufferStaticPs4(uint32_t bufferSize)
:	VertexBufferPs4(bufferSize)
{
}

	}
}
