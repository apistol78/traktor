/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Script/Lua/ScriptBlobLua.h"

namespace traktor::script
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.ScriptBlobLua", 0, ScriptBlobLua, IScriptBlob)

void ScriptBlobLua::serialize(ISerializer& s)
{
	s >> Member< std::string >(L"fileName", m_fileName);
	s >> Member< std::string >(L"script", m_script);
}

}
