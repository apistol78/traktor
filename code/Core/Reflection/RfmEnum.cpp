/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Reflection/RfmEnum.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.RfmEnum", RfmEnum, ReflectionMember)

RfmEnum::RfmEnum(const wchar_t* name, const std::wstring& value)
:	ReflectionMember(name)
,	m_value(value)
{
}

bool RfmEnum::replace(const ReflectionMember* source)
{
	if (const RfmEnum* sourceEnum = dynamic_type_cast< const RfmEnum* >(source))
	{
		m_value = sourceEnum->m_value;
		return true;
	}
	else
		return false;
}

}
