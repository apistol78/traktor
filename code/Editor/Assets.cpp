/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Editor/Assets.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberStl.h"

namespace traktor::editor
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.editor.Assets", 1, Assets, ISerializable)

void Assets::serialize(ISerializer& s)
{
	T_FATAL_ASSERT(s.getVersion() >= 1);
	s >> MemberStlVector< Dependency, MemberComposite< Dependency > >(L"dependencies", m_dependencies);
}

void Assets::Dependency::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"id", id);
	s >> Member< bool >(L"editorDeployOnly", editorDeployOnly);
}

}
