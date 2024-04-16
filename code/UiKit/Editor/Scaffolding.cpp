/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/IRuntimeClass.h"
#include "Core/Serialization/ISerializer.h"
#include "Resource/Member.h"
#include "UiKit/Editor/Scaffolding.h"

namespace traktor::uikit
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.uikit.Scaffolding", 0, Scaffolding, ISerializable)

void Scaffolding::serialize(ISerializer& s)
{
	s >> resource::Member< IRuntimeClass >(L"scaffoldingClass", m_scaffoldingClass);
}

}
