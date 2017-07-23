/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <Core/Serialization/ISerializer.h>
#include <Core/Serialization/Member.h>
#include <Core/Serialization/MemberRef.h>
#include "Ddc/DfnAlias.h"

using namespace traktor;

namespace ddc
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"ddc.DfnAlias", 0, DfnAlias, DfnNode)

DfnAlias::DfnAlias()
{
}

DfnAlias::DfnAlias(
	const std::wstring& language,
	DfnNode* type,
	const std::wstring& languageType,
	const std::wstring& languageArgType,
	const std::wstring& languageMember
)
:	m_language(language)
,	m_type(type)
,	m_languageType(languageType)
,	m_languageArgType(languageArgType)
,	m_languageMember(languageMember)
{
}

bool DfnAlias::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"language", m_language);
	s >> MemberRef< DfnNode >(L"type", m_type);
	s >> Member< std::wstring >(L"languageType", m_languageType);
	s >> Member< std::wstring >(L"languageArgType", m_languageArgType);
	s >> Member< std::wstring >(L"languageMember", m_languageMember);
	return DfnNode::serialize(s);
}

}
