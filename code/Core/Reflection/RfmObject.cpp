/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Reflection/RfmObject.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.RfmObject", RfmObject, ReflectionMember)

RfmObject::RfmObject(const wchar_t* name, ISerializable* value)
:	ReflectionMember(name)
,	m_value(value)
{
}

bool RfmObject::replace(const ReflectionMember* source)
{
	if (const RfmObject* sourceObject = dynamic_type_cast< const RfmObject* >(source))
	{
		m_value = sourceObject->m_value;
		return true;
	}
	else
		return false;
}

}
