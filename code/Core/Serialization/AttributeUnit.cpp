#include "Core/Serialization/AttributeUnit.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.AttributeUnit", AttributeUnit, Attribute)

AttributeUnit::AttributeUnit(UnitType unit)
:	m_unit(unit)
{
}

UnitType AttributeUnit::getUnit() const
{
	return m_unit;
}

}
