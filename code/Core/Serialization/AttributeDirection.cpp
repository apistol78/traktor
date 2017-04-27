/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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
