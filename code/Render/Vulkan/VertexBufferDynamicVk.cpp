#include "Render/Vulkan/VertexBufferDynamicVk.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VertexBufferDynamicVk", VertexBufferDynamicVk, VertexBufferVk)

Ref< VertexBufferDynamicVk > VertexBufferDynamicVk::create(
	uint32_t bufferSize,
	const std::vector< VertexElement >& vertexElements
)
{
	Ref< VertexBufferDynamicVk > vb = new VertexBufferDynamicVk(bufferSize);
	return vb;
}

VertexBufferDynamicVk::~VertexBufferDynamicVk()
{
	destroy();
}

void VertexBufferDynamicVk::destroy()
{
}

void* VertexBufferDynamicVk::lock()
{
	return 0;
}

void* VertexBufferDynamicVk::lock(uint32_t vertexOffset, uint32_t vertexCount)
{
	T_FATAL_ERROR;
	return 0;
}

void VertexBufferDynamicVk::unlock()
{
}

void VertexBufferDynamicVk::prepare()
{
	VertexBufferVk::prepare();
}

VertexBufferDynamicVk::VertexBufferDynamicVk(uint32_t bufferSize)
:	VertexBufferVk(bufferSize)
{
}

	}
}
