/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/AttributeType.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.AttributeType", AttributeType, Attribute)

AttributeType::AttributeType(const TypeInfo& memberType)
:	m_memberType(memberType)
{
}

const TypeInfo& AttributeType::getMemberType() const
{
	return m_memberType;
}

}
