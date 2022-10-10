#include "Render/Buffer.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.Buffer", Buffer, Object)

uint32_t Buffer::getElementCount() const
{
	return m_elementCount;
}

uint32_t Buffer::getElementSize() const
{
	return m_elementSize;
}

uint32_t Buffer::getBufferSize() const
{
	return m_elementCount * m_elementSize;
}

Buffer::Buffer(uint32_t elementCount, uint32_t elementSize)
:	m_elementCount(elementCount)
,	m_elementSize(elementSize)
{
}

}
