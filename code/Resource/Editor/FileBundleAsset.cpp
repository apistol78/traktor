/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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
