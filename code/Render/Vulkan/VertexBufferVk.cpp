#include "Render/Vulkan/VertexBufferVk.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VertexBufferVk", VertexBufferVk, VertexBuffer)

void VertexBufferVk::prepare()
{
}

VertexBufferVk::VertexBufferVk(uint32_t bufferSize)
:	VertexBuffer(bufferSize)
{
}

	}
}
