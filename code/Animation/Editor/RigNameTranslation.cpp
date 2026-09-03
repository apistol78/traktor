/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Editor/RigNameTranslation.h"

#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberSmallMap.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.animation.RigNameTranslation", 0, RigNameTranslation, ISerializable)

std::wstring RigNameTranslation::translate(const std::wstring& name) const
{
	const auto it = m_translations.find(name);
	return it != m_translations.end() ? it->second : name;
}

void RigNameTranslation::serialize(ISerializer& s)
{
	s >> MemberSmallMap< std::wstring, std::wstring >(L"translations", m_translations);
}

}
