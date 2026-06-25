/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "MCP/Server/Editor/SkillAsset.h"

#include "Core/Serialization/AttributeMultiLine.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRefArray.h"

namespace traktor::mcp
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mcp.SkillParameter", 0, SkillParameter, ISerializable)

void SkillParameter::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", m_name);
	s >> Member< std::wstring >(L"description", m_description);
	s >> Member< std::wstring >(L"defaultValue", m_defaultValue);
}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mcp.SkillAsset", 0, SkillAsset, ISerializable)

void SkillAsset::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", m_name);
	s >> Member< std::wstring >(L"description", m_description);
	s >> Member< std::wstring >(L"whenToUse", m_whenToUse, AttributeMultiLine());
	s >> Member< std::wstring >(L"body", m_body, AttributeMultiLine());
	s >> Member< std::wstring >(L"engineVersion", m_engineVersion);
	s >> Member< bool >(L"published", m_published);
	s >> MemberRefArray< SkillParameter >(L"parameters", m_parameters);
}

}
