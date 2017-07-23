/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <Core/Serialization/ISerializer.h>
#include <Core/Serialization/Member.h>
#include "Ddc/DfnTypeSubst.h"

using namespace traktor;

namespace ddc
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"ddc.DfnTypeSubst", 0, DfnTypeSubst, DfnNode)

DfnTypeSubst::DfnTypeSubst()
{
}

DfnTypeSubst::DfnTypeSubst(const std::wstring& tag)
:	m_tag(tag)
{
}

bool DfnTypeSubst::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"tag", m_tag);
	return DfnNode::serialize(s);
}

}
