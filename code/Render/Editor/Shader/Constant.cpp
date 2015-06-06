#include <algorithm>
#include <cmath>
#include "Core/Math/Const.h"
#include "Render/Editor/Shader/Constant.h"

namespace traktor
{
	namespace render
	{

Constant::Constant()
:	m_type(PntVoid)
{
	for (int32_t i = 0; i < 4; ++i)
	{
		m_const[i] = false;
		m_value[i] = 0.0f;
	}
}

Constant::Constant(PinType type)
:	m_type(type)
{
	for (int32_t i = 0; i < 4; ++i)
	{
		m_const[i] = false;
		m_value[i] = 0.0f;
	}
}

Constant::Constant(float x)
:	m_type(PntScalar1)
{
	m_const[0] = true;
	m_value[0] = x;
	m_const[1] = false;
	m_value[1] = 0.0f;
	m_const[2] = false;
	m_value[2] = 0.0f;
	m_const[3] = false;
	m_value[3] = 0.0f;
}

Constant::Constant(float x, float y, float z, float w)
:	m_type(PntScalar4)
{
	m_const[0] = true;
	m_value[0] = x;
	m_const[1] = true;
	m_value[1] = y;
	m_const[2] = true;
	m_value[2] = z;
	m_const[3] = true;
	m_value[3] = w;
}

Constant Constant::cast(PinType type) const
{
	Constant out(type);
	if (m_type == PntScalar1)
	{
		for (int32_t i = 0; i < getPinTypeWidth(type); ++i)
		{
			out.m_const[i] = m_const[0];
			out.m_value[i] = m_value[0];
		}
	}
	else
	{
		int32_t width = getPinTypeWidth(std::min(type, m_type));
		for (int32_t i = 0; i < width; ++i)
		{
			out.m_const[i] = m_const[i];
			out.m_value[i] = m_value[i];
		}
		for (int32_t i = width; i < getPinTypeWidth(type); ++i)
		{
			out.m_const[i] = true;
			out.m_value[i] = 0.0f;
		}
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

void Constant::setVariant(int32_t index)
{
	m_const[index] = false;
	m_value[index] = 0.0f;
}

void Constant::setValue(int32_t index, float value)
{
	m_const[index] = true;
	m_value[index] = value;
}

float Constant::getValue(int32_t index) const
{
	T_ASSERT_M (m_const[index], L"Cannot get value of non-const element");
	return m_value[index];
}

bool Constant::isAllConst() const
{
	for (int32_t i = 0; i < getWidth(); ++i)
	{
		if (!isConst(i))
			return false;
	}
	return true;
}

bool Constant::isAnyConst() const
{
	for (int32_t i = 0; i < getWidth(); ++i)
	{
		if (isConst(i))
			return true;
	}
	return false;
}

bool Constant::isConst(int32_t index) const
{
	return m_const[index];
}

bool Constant::isAllZero() const
{
	for (int32_t i = 0; i < getWidth(); ++i)
	{
		if (!isZero(i))
			return false;
	}
	return true;
}

bool Constant::isZero(int32_t index) const
{
	return m_const[index] && fabs(m_value[index]) <= FUZZY_EPSILON;
}

bool Constant::isAllOne() const
{
	for (int32_t i = 0; i < getWidth(); ++i)
	{
		if (!isOne(i))
			return false;
	}
	return true;
}

bool Constant::isOne(int32_t index) const
{
	return m_const[index] && fabs(m_value[index] - 1.0f) <= FUZZY_EPSILON;
}

bool Constant::operator == (const Constant& rh) const
{
	if (getWidth() != rh.getWidth())
		return false;

	for (int32_t i = 0; i < getWidth(); ++i)
	{
		if (m_const[i] != rh.m_const[i])
			return false;

		if (fabs(m_value[i] - rh.m_value[i]) >= FUZZY_EPSILON)
			return false;
	}

	return true;
}

bool Constant::operator != (const Constant& rh) const
{
	return !(*this == rh);
}

	}
}
