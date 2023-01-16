/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberStl.h"
#include "Script/IScriptBlob.h"
#include "Script/ScriptResource.h"

namespace traktor::script
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.ScriptResource", 0, ScriptResource, ISerializable)

void ScriptResource::serialize(ISerializer& s)
{
	s >> MemberStlVector< Guid >(L"dependencies", m_dependencies);
	s >> MemberRef< const IScriptBlob >(L"blob", m_blob);
}

}
