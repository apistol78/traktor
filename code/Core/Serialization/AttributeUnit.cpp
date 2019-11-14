#include "Core/Serialization/AttributeUnit.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.AttributeUnit", AttributeUnit, Attribute)

AttributeUnit::AttributeUnit(UnitType unit, bool perSecond)
:	m_unit(unit)
,	m_perSecond(perSecond)
{
}

UnitType AttributeUnit::getUnit() const
{
	return m_unit;
}

bool AttributeUnit::getPerSecond() const
{
	return m_perSecond;
}

}
