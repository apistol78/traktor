/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <Core/Serialization/ISerializer.h>
#include <Core/Serialization/Member.h>
#include <Core/Serialization/MemberRef.h>
#include "Ddc/DfnNamespace.h"

using namespace traktor;

namespace ddc
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"ddc.DfnNamespace", 0, DfnNamespace, DfnNode)

DfnNamespace::DfnNamespace()
{
}

DfnNamespace::DfnNamespace(const std::wstring& name, DfnNode* statements)
:	m_name(name)
,	m_statements(statements)
{
}

bool DfnNamespace::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", m_name);
	s >> MemberRef< DfnNode >(L"statements", m_statements);
	return DfnNode::serialize(s);
}

}
