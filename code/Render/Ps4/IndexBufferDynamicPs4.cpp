/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/Ps4/ContextPs4.h"
#include "Render/Ps4/IndexBufferDynamicPs4.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.IndexBufferDynamicPs4", IndexBufferDynamicPs4, IndexBufferPs4)

Ref< IndexBufferDynamicPs4 > IndexBufferDynamicPs4::create(ContextPs4* context, IndexType indexType, uint32_t bufferSize)
{
	Ref< IndexBufferDynamicPs4 > ib = new IndexBufferDynamicPs4(indexType, bufferSize);
	ib->m_context = context;
	return ib;
}

IndexBufferDynamicPs4::~IndexBufferDynamicPs4()
{
	destroy();
}

void IndexBufferDynamicPs4::destroy()
{
}

void* IndexBufferDynamicPs4::lock()
{
	return 0;
}

void IndexBufferDynamicPs4::unlock()
{
}

void IndexBufferDynamicPs4::prepare()
{
	IndexBufferPs4::prepare();
}

IndexBufferDynamicPs4::IndexBufferDynamicPs4(IndexType indexType, uint32_t bufferSize)
:	IndexBufferPs4(indexType, bufferSize)
{
}

	}
}
