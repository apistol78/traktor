#include "Heightfield/MaterialMask.h"

namespace traktor
{
	namespace hf
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.hf.MaterialMask", MaterialMask, Object)

MaterialMask::MaterialMask(uint32_t size)
:	m_size(size)
,	m_data(new uint8_t [size * size])
{
}

MaterialMask::~MaterialMask()
{
	delete[] m_data;
}

uint32_t MaterialMask::getSize() const
{
	return m_size;
}

uint8_t MaterialMask::getMaterial(int x, int y) const
{
	T_ASSERT (x >= 0 && x < m_size);
	T_ASSERT (y >= 0 && y < m_size);
	return m_data[x + y * m_size];
}

	}
}
