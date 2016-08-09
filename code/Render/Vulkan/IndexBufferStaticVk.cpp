#include "Render/Vulkan/IndexBufferStaticVk.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.IndexBufferStaticVk", IndexBufferStaticVk, IndexBufferVk)

Ref< IndexBufferStaticVk > IndexBufferStaticVk::create(IndexType indexType, uint32_t bufferSize)
{
	Ref< IndexBufferStaticVk > ib = new IndexBufferStaticVk(indexType, bufferSize);
	return ib;
}

IndexBufferStaticVk::~IndexBufferStaticVk()
{
	destroy();
}

void IndexBufferStaticVk::destroy()
{
}

void* IndexBufferStaticVk::lock()
{
	return 0;
}

void IndexBufferStaticVk::unlock()
{
}

void IndexBufferStaticVk::prepare()
{
	IndexBufferVk::prepare();
}

IndexBufferStaticVk::IndexBufferStaticVk(IndexType indexType, uint32_t bufferSize)
:	IndexBufferVk(indexType, bufferSize)
{
}

	}
}
