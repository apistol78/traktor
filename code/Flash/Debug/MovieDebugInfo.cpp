/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"
#include "Flash/FlashMovie.h"
#include "Flash/Debug/MovieDebugInfo.h"

namespace traktor
{
	namespace flash
	{
	
T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.MovieDebugInfo", 0, MovieDebugInfo, ISerializable)

MovieDebugInfo::MovieDebugInfo()
{
}

MovieDebugInfo::MovieDebugInfo(const std::wstring& name, const FlashMovie* movie)
:	m_name(name)
,	m_movie(movie)
{
}

void MovieDebugInfo::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", m_name);
	s >> MemberRef< const FlashMovie >(L"movie", m_movie);
}

	}
}
