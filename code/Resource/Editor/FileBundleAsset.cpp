/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberStl.h"
#include "Resource/Editor/FileBundleAsset.h"

namespace traktor
{
	namespace resource
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.resource.FileBundleAsset", 0, FileBundleAsset, ISerializable)

const std::list< FileBundleAsset::Pattern >& FileBundleAsset::getPatterns() const
{
	return m_patterns;
}

void FileBundleAsset::serialize(ISerializer& s)
{
	s >> MemberStlList< Pattern, MemberComposite< Pattern > >(L"patterns", m_patterns);
}

FileBundleAsset::Pattern::Pattern()
:	recursive(false)
{
}

void FileBundleAsset::Pattern::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"sourceBase", sourceBase);
	s >> Member< std::wstring >(L"outputBase", outputBase);
	s >> Member< std::wstring >(L"sourceMask", sourceMask);
	s >> Member< bool >(L"recursive", recursive);
}

	}
}
