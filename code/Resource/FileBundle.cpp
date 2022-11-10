/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/String.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Resource/FileBundle.h"

namespace traktor
{
	namespace resource
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.resource.FileBundle", 0, FileBundle, ISerializable)

std::wstring FileBundle::lookup(const std::wstring& id) const
{
	auto it = m_dataIds.find(toLower(id));
	return it != m_dataIds.end() ? it->second : L"";
}

void FileBundle::serialize(ISerializer& s)
{
	s >> MemberStlMap< std::wstring, std::wstring >(L"dataIds", m_dataIds);
}

	}
}
