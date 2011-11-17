#include "Heightfield/MaterialMask.h"

namespace traktor
{
	namespace hf
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.hf.MaterialMask", MaterialMask, Object)

MaterialMask::MaterialMask(uint32_t size, const RefArray< MaterialParams >& params)
:	m_size(size)
,	m_data(new uint8_t [size * size])
,	m_params(params)
{
}

uint32_t MaterialMask::getSize() const
{
	return m_size;
}

uint8_t MaterialMask::getId(int x, int y) const
{
	T_ASSERT (x >= 0 && x < m_size);
	T_ASSERT (y >= 0 && y < m_size);
	return m_data[x + y * m_size];
}

const MaterialParams* MaterialMask::getParams(uint8_t id) const
{
	T_ASSERT (id < m_params.size());
	return m_params[id];
}

const MaterialParams* MaterialMask::getParams(int x, int y) const
{
	uint8_t id = getId(x, y);
	return getParams(id);
}

	}
}
