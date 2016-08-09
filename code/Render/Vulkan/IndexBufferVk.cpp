#include "Render/Vulkan/IndexBufferVk.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.IndexBufferVk", IndexBufferVk, IndexBuffer)

IndexBufferVk::IndexBufferVk(IndexType indexType, uint32_t bufferSize)
:	IndexBuffer(indexType, bufferSize)
{
}

void IndexBufferVk::prepare()
{
}

	}
}
