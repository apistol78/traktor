#include "Render/Vulkan/VertexBufferStaticVk.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VertexBufferStaticVk", VertexBufferStaticVk, VertexBufferVk)

Ref< VertexBufferStaticVk > VertexBufferStaticVk::create(
	uint32_t bufferSize,
	const std::vector< VertexElement >& vertexElements
)
{
	Ref< VertexBufferStaticVk > vb = new VertexBufferStaticVk(bufferSize);
	return vb;
}

VertexBufferStaticVk::~VertexBufferStaticVk()
{
	destroy();
}

void VertexBufferStaticVk::destroy()
{
}

void* VertexBufferStaticVk::lock()
{
	return 0;
}

void* VertexBufferStaticVk::lock(uint32_t vertexOffset, uint32_t vertexCount)
{
	T_FATAL_ERROR;
	return 0;
}

void VertexBufferStaticVk::unlock()
{
	setContentValid(true);
}

void VertexBufferStaticVk::prepare()
{
	VertexBufferVk::prepare();
}

VertexBufferStaticVk::VertexBufferStaticVk(uint32_t bufferSize)
:	VertexBufferVk(bufferSize)
{
}

	}
}
