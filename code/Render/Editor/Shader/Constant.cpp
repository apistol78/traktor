#include "Core/Math/Const.h"
#include "Render/Editor/Shader/Constant.h"

namespace traktor
{
	namespace render
	{

Constant::Constant()
:	m_type(PntVoid)
{
	for (int32_t i = 0; i < sizeof_array(m_data); ++i)
		m_data[i] = 0.0f;
}

Constant::Constant(PinType type)
:	m_type(type)
{
	for (int32_t i = 0; i < sizeof_array(m_data); ++i)
		m_data[i] = 0.0f;
}

Constant::Constant(float scalar1)
:	m_type(PntScalar1)
{
	m_data[0] = scalar1;
	for (int32_t i = 1; i < sizeof_array(m_data); ++i)
		m_data[i] = 0.0f;
}

Constant Constant::cast(PinType type) const
{
	Constant out(type);
	if (m_type == PntScalar1)
	{
		for (int32_t i = 0; i < getPinTypeWidth(type); ++i)
			out[i] = m_data[0];
	}
	else
	{
		for (int32_t i = 0; i < getPinTypeWidth(std::min(type, m_type)); ++i)
			out[i] = m_data[i];
	}
	return out;
}

PinType Constant::getType() const
{
	return m_type;
}

int32_t Constant::getWidth() const
{
	return getPinTypeWidth(m_type);
}

bool Constant::isZero() const
{
	for (int32_t i = 0; i < getWidth(); ++i)
	{
		if (std::abs(m_data[i]) >= FUZZY_EPSILON)
			return false;
	}
	return true;
}

float& Constant::operator [] (int32_t index)
{
	T_ASSERT (index < sizeof_array(m_data));
	return m_data[index];
}

float Constant::operator [] (int32_t index) const
{
	T_ASSERT (index < sizeof_array(m_data));
	return m_data[index];
}

	}
}
