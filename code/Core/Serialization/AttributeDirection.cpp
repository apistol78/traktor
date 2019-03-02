#include "Core/Serialization/AttributeDirection.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.AttributeDirection", AttributeDirection, Attribute)

AttributeDirection::AttributeDirection()
:	m_unit(false)
{
}

AttributeDirection::AttributeDirection(bool unit)
:	m_unit(unit)
{
}

}
