/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/Ps4/ContextPs4.h"
#include "Render/Ps4/IndexBufferStaticPs4.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.IndexBufferStaticPs4", IndexBufferStaticPs4, IndexBufferPs4)

Ref< IndexBufferStaticPs4 > IndexBufferStaticPs4::create(ContextPs4* context, IndexType indexType, uint32_t bufferSize)
{
	Ref< IndexBufferStaticPs4 > ib = new IndexBufferStaticPs4(indexType, bufferSize);
	ib->m_context = context;
	return ib;
}

IndexBufferStaticPs4::~IndexBufferStaticPs4()
{
	destroy();
}

void IndexBufferStaticPs4::destroy()
{
}

void* IndexBufferStaticPs4::lock()
{
	return 0;
}

void IndexBufferStaticPs4::unlock()
{
}

void IndexBufferStaticPs4::prepare()
{
	IndexBufferPs4::prepare();
}

IndexBufferStaticPs4::IndexBufferStaticPs4(IndexType indexType, uint32_t bufferSize)
:	IndexBufferPs4(indexType, bufferSize)
{
}

	}
}
