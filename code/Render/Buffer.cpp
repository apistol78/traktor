#include "Render/Buffer.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.Buffer", Buffer, Object)

uint32_t Buffer::getBufferSize() const
{
	return m_bufferSize;
}

Buffer::Buffer(uint32_t bufferSize)
:	m_bufferSize(bufferSize)
{
}

	}
}
