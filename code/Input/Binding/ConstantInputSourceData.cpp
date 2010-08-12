#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Input/Binding/ConstantInputSource.h"
#include "Input/Binding/ConstantInputSourceData.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.ConstantInputSourceData", 0, ConstantInputSourceData, IInputSourceData)

ConstantInputSourceData::ConstantInputSourceData()
:	m_value(0.0f)
{
}

ConstantInputSourceData::ConstantInputSourceData(float value)
:	m_value(value)
{
}

Ref< IInputSource > ConstantInputSourceData::createInstance() const
{
	return new ConstantInputSource(m_value);
}

bool ConstantInputSourceData::serialize(ISerializer& s)
{
	return s >> Member< float >(L"value", m_value);
}
	
	}
}
