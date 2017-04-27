/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <string>
#include "Core/Serialization/MemberComplex.h"

namespace traktor
{

MemberComplex::MemberComplex(const wchar_t* const name, bool compound)
:	m_name(name)
,	m_compound(compound)
{
}

MemberComplex::~MemberComplex()
{
}

const wchar_t* const MemberComplex::getName() const
{
	return m_name;
}

bool MemberComplex::getCompound() const
{
	return m_compound;
}

}
