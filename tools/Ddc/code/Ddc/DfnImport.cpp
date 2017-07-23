/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <Core/Serialization/ISerializer.h>
#include <Core/Serialization/Member.h>
#include "Ddc/DfnImport.h"

using namespace traktor;

namespace ddc
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"ddc.DfnImport", 0, DfnImport, DfnNode)

DfnImport::DfnImport()
{
}

DfnImport::DfnImport(const std::wstring& module)
:	m_module(module)
{
}

bool DfnImport::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"module", m_module);
	return DfnNode::serialize(s);
}

}
