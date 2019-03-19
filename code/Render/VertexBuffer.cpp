#include "Render/VertexBuffer.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VertexBuffer", VertexBuffer, Object)

uint32_t VertexBuffer::getBufferSize() const
{
	return m_bufferSize;
}

VertexBuffer::VertexBuffer(uint32_t bufferSize)
:	m_bufferSize(bufferSize)
{
}

	}
}
