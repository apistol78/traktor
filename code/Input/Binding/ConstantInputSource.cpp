#include "Core/Misc/String.h"
#include "Input/Binding/ConstantInputSource.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.ConstantInputSource", ConstantInputSource, Object)

ConstantInputSource::ConstantInputSource(float value)
:	m_value(value)
{
}

std::wstring ConstantInputSource::getDescription() const
{
	return toString(m_value);
}

float ConstantInputSource::read(float T, float dT)
{
	return m_value;
}

	}
}
