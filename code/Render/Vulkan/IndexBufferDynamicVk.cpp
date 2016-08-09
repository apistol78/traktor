#include "Render/Vulkan/IndexBufferDynamicVk.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.IndexBufferDynamicVk", IndexBufferDynamicVk, IndexBufferVk)

Ref< IndexBufferDynamicVk > IndexBufferDynamicVk::create(IndexType indexType, uint32_t bufferSize)
{
	Ref< IndexBufferDynamicVk > ib = new IndexBufferDynamicVk(indexType, bufferSize);
	return ib;
}

IndexBufferDynamicVk::~IndexBufferDynamicVk()
{
	destroy();
}

void IndexBufferDynamicVk::destroy()
{
}

void* IndexBufferDynamicVk::lock()
{
	return 0;
}

void IndexBufferDynamicVk::unlock()
{
}

void IndexBufferDynamicVk::prepare()
{
	IndexBufferVk::prepare();
}

IndexBufferDynamicVk::IndexBufferDynamicVk(IndexType indexType, uint32_t bufferSize)
:	IndexBufferVk(indexType, bufferSize)
{
}

	}
}
