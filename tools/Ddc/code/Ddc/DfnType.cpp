/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <Core/Serialization/ISerializer.h>
#include <Core/Serialization/Member.h>
#include <Core/Serialization/MemberRef.h>
#include "Ddc/DfnType.h"

using namespace traktor;

namespace ddc
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"ddc.DfnType", 0, DfnType, DfnNode)

DfnType::DfnType()
:	m_isArray(false)
,	m_range(0)
{
}

DfnType::DfnType(const std::wstring& name, DfnNode* subst, bool isArray, int32_t range)
:	m_name(name)
,	m_subst(subst)
,	m_isArray(isArray)
,	m_range(range)
{
}

bool DfnType::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", m_name);
	s >> MemberRef< DfnNode >(L"subst", m_subst);
	s >> Member< bool >(L"isArray", m_isArray);
	s >> Member< int32_t >(L"range", m_range);
	return DfnNode::serialize(s);
}

}
