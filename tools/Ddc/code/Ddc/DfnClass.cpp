/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <Core/Serialization/ISerializer.h>
#include <Core/Serialization/Member.h>
#include <Core/Serialization/MemberEnum.h>
#include <Core/Serialization/MemberRef.h>
#include "Ddc/DfnClass.h"

using namespace traktor;

namespace ddc
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"ddc.DfnClass", 0, DfnClass, DfnNode)

DfnClass::DfnClass()
:	m_access(AccPrivate)
{
}

DfnClass::DfnClass(Access access, const std::wstring& name, DfnNode* members)
:	m_access(access)
,	m_name(name)
,	m_members(members)
{
}

DfnClass::DfnClass(Access access, const std::wstring& name, const std::wstring& super, DfnNode* members)
:	m_access(access)
,	m_name(name)
,	m_super(super)
,	m_members(members)
{
}

bool DfnClass::serialize(ISerializer& s)
{
	const MemberEnum< Access >::Key c_Access_keys[] =
	{
		{ L"AccPrivate", AccPrivate },
		{ L"AccPublic", AccPublic },
		0
	};
	s >> MemberEnum< Access >(L"access", m_access, c_Access_keys);
	s >> Member< std::wstring >(L"name", m_name);
	s >> Member< std::wstring >(L"super", m_super);
	s >> MemberRef< DfnNode >(L"members", m_members);
	return DfnNode::serialize(s);
}

}
