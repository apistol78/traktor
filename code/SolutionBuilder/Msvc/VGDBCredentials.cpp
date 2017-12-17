/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "SolutionBuilder/Msvc/VGDBCredentials.h"

namespace traktor
{
	namespace sb
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"VGDBCredentials", 0, VGDBCredentials, ISerializable)

void VGDBCredentials::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"host", m_host);
	s >> Member< std::wstring >(L"user", m_user);
	s >> Member< std::wstring >(L"localPath", m_localPath);
	s >> Member< std::wstring >(L"remotePath", m_remotePath);
}

	}
}
