#include "Render/Dx11/VertexBufferDx11.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VertexBufferDx11", VertexBufferDx11, VertexBuffer)

VertexBufferDx11::VertexBufferDx11(uint32_t bufferSize)
:	VertexBuffer(bufferSize)
{
}

	}
}
