#include "Render/StructBuffer.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.StructBuffer", StructBuffer, Object)

uint32_t StructBuffer::getBufferSize() const
{
	return m_bufferSize;
}

StructBuffer::StructBuffer(uint32_t bufferSize)
:	m_bufferSize(bufferSize)
{
}

	}
}
