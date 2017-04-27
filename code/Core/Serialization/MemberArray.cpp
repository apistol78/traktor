/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/MemberArray.h"

namespace traktor
{

MemberArray::MemberArray(const wchar_t* const name, const Attribute* attributes)
:	m_name(name)
,	m_attributes(attributes)
{
}

}
